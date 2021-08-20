#include <dpn/input/from_file.hpp>
#include <dpn/util.hpp>
#include <dpn/log.hpp>
#include <gubg/file/system.hpp>
#include <gubg/string_algo/algo.hpp>
#include <gubg/mss.hpp>
#include <cassert>

namespace dpn { namespace input { 

    bool append_from_file(onto::Nodes &nodes, const std::string &filepath)
    {
        MSS_BEGIN(bool);

        std::string content;
        MSS(gubg::file::read(content, filepath));

        MSS(append_from_string(nodes, content, filepath));

        MSS_END();
    }

    bool append_from_string(onto::Nodes &nodes, const std::string &content, const std::string &filepath)
    {
        MSS_BEGIN(bool);

        std::vector<std::string> lines;
        gubg::string_algo::split_lines(lines, content);

        onto::Node *title_ptr = nullptr;
        metadata::Metadata *metadata_ptr = nullptr;
        unsigned int empty_count = 0u;

        onto::Nodes title_nodes;
        metadata::Metadata metadata;

        std::string line;

        std::vector<metadata::Item> metadata_items;

        for (auto &raw_line: lines)
        {
            if (raw_line.starts_with("---"))
            {
                //Switch from reading Node data to reading Metadata
                metadata_ptr = &metadata;
                title_ptr = nullptr;
                continue;
            }

            auto process_empty_count = [&](){
                if (title_ptr)
                {
                    //Add the collected empty lines
                    for (auto ix = 0u; ix < empty_count; ++ix)
                        title_ptr->childs.emplace_back(onto::Type::Line);
                    empty_count = 0u;
                }
            };

            onto::Node *node_ptr = nullptr;
            if (metadata_ptr)
            {
            }
            else if (const auto pair = util::lead_count('#', ' ', raw_line); pair.first > 0)
            {
                //We found a Title

                process_empty_count();

                title_nodes.emplace_back(onto::Type::Title);
                title_ptr = &title_nodes.back();
                node_ptr = title_ptr;
                title_ptr->depth = pair.first;
                line = raw_line.substr(pair.first+pair.second);
            }
            else if (raw_line.substr(0, 2) == "@[")
            {
                //We found a Link

                if (title_ptr)
                {
                    title_ptr->childs.emplace_back(onto::Type::Link);
                    node_ptr = &title_ptr->childs.back();
                    line = raw_line.substr(1, std::string::npos);
                }
            }
            else
            {
                if (!title_ptr)
                {
                    log::warning() << "Dropping raw_line `" << raw_line << "`, there is no Node to add it to" << std::endl;
                    continue;
                }

                if (raw_line.empty())
                {
                    empty_count++;
                }
                else
                {
                    process_empty_count();

                    //Add the actual raw_line
                    title_ptr->childs.emplace_back(onto::Type::Line);
                    node_ptr = &title_ptr->childs.back();
                    line = raw_line;
                }
            }

            if (node_ptr)
            {
                metadata::split(node_ptr->text, metadata_items, line);
                node_ptr->metadata.setup(metadata_items);

                if (node_ptr->type == onto::Type::Link)
                {
                    //TODO: extract link text and recurse parsing the file for Type::Link
                }
            }
        }

        MSS(standardize_depths_(title_nodes));

        //Append nodes from title_nodes to nodes
        //but nest them according to depth
        std::vector<onto::Nodes *> depth0__nodes = {&nodes};
        for (const auto &s: title_nodes)
        {
            //Create zero-based depth that we can use to index depth0__nodes
            MSS(s.depth > 0);
            const auto depth0 = s.depth-1;

            if (depth0 == depth0__nodes.size())
            {
                //The nodes at this depth0 is not present yet
                MSS(!depth0__nodes[depth0-1]->empty());
                auto &parent = depth0__nodes[depth0-1]->back();
                depth0__nodes.push_back(&parent.childs);
            }
            MSS(depth0 < depth0__nodes.size());

            depth0__nodes.resize(depth0+1);
            depth0__nodes[depth0]->push_back(s);

            if (depth0 == 0)
                depth0__nodes[depth0]->back().filepath = filepath;
        }

        //Aggregate metadata for all root nodes
        for (auto &node: nodes)
            node.aggregate_metadata(nullptr);

        MSS_END();
    }

    //Rework the text depths to ensure child nodes are exactly one level deeper
    bool standardize_depths_(onto::Nodes &nodes)
    {
        MSS_BEGIN(bool);

        unsigned int level = 0u;
        for (auto ix = 0u; ix < nodes.size(); ++ix)
        {
            auto &s = nodes[ix];
            MSS(s.depth > 0);

            if (s.depth < level)
            {
                //This node is more towards the root: OK
            }
            else if (s.depth == level)
            {
                //This node is at the same level: this is a child
            }
            else if (s.depth == level+1)
            {
                //This node is one level deeper: this is a child
                ++level;
            }
            else if (s.depth > level+1)
            {
                //We only accept a unit step when going deeper
                //We will move all affected nodes to the root
                const unsigned int shift_count = s.depth-level-1;
                for (auto ix2 = ix; ix2 < nodes.size(); ++ix2)
                {
                    auto &s2 = nodes[ix2];
                    if (s2.depth >= s.depth)
                    {
                        s2.depth -= shift_count;
                        log::warning() << "Shifting node " << s2.text << " by " << shift_count << " deeper to level " << s2.depth << std::endl;
                    }
                }
                ++level;
            }
        }

        MSS_END();
    }

} }
