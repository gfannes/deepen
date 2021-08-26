#include <dpn/input/from_file.hpp>
#include <dpn/util.hpp>
#include <dpn/log.hpp>
#include <gubg/file/system.hpp>
#include <gubg/string_algo/algo.hpp>
#include <gubg/mss.hpp>
#include <cassert>

namespace dpn { namespace input { 

    bool load_from_file(onto::Node &file_node, const std::filesystem::path &filepath)
    {
        MSS_BEGIN(bool);

        std::string content;
        MSS(gubg::file::read(content, filepath), log::error() << "Could not read content from `" << filepath << "`" << std::endl);

        MSS(load_from_string(file_node, content, filepath));

        MSS_END();
    }

    bool load_from_string(onto::Node &file_node, const std::string &content, const std::filesystem::path &filepath)
    {
        MSS_BEGIN(bool);

        file_node.type = onto::Type::File;
        file_node.filepath = filepath;

        std::vector<std::string> lines;
        gubg::string_algo::split_lines(lines, content);

        onto::Node *title_ptr = &file_node;
        unsigned int empty_count = 0u;

        onto::Nodes title_nodes;

        std::string line;

        std::vector<metadata::Item> metadata_items;

        for (auto &raw_line: lines)
        {
            assert(!!title_ptr);

            gubg::Strange strange{raw_line};
            gubg::Strange tmp;

            auto process_empty_count = [&](){
                MSS_BEGIN(bool);
                //Add the collected empty lines
                for (auto ix = 0u; ix < empty_count; ++ix)
                {
                    MSS(!!title_ptr, log::internal_error() << "title_ptr should never be nullptr" << std::endl);
                    title_ptr->childs.emplace_back(onto::Type::Line);
                }
                empty_count = 0u;
                MSS_END();
            };


            onto::Node *node_ptr = nullptr;
            if (const auto pair = util::lead_count('#', ' ', raw_line); pair.first > 0)
            {
                //We found a Title

                MSS(process_empty_count());

                title_nodes.emplace_back(onto::Type::Title);
                title_ptr = &title_nodes.back();
                node_ptr = title_ptr;
                title_ptr->depth = pair.first;
                line = raw_line.substr(pair.first+pair.second);
            }
            else
            {
                MSS(!!title_ptr, log::internal_error() << "title_ptr should never be nullptr" << std::endl);

                if (raw_line.empty())
                {
                    empty_count++;
                }
                else
                {
                    MSS(process_empty_count());

                    //Add the actual raw_line
                    title_ptr->childs.emplace_back(onto::Type::Line);
                    node_ptr = &title_ptr->childs.back();
                    line = raw_line;
                }
            }

            if (node_ptr)
            {
                metadata::split(node_ptr->text, metadata_items, line);
                node_ptr->metadata.setup(metadata_items, filepath.parent_path());

                if (node_ptr->type == onto::Type::File)
                {
                    //TODO: extract link text and recurse parsing the file for Type::File
                }
            }
        }

        MSS(standardize_depths_(title_nodes));

        //Append nodes from title_nodes to nodes
        //but nest them according to depth
        std::vector<onto::Nodes *> depth0__nodes = {&file_node.childs};
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
        }

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
