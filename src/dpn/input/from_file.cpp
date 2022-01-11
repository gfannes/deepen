#include <dpn/input/from_file.hpp>
#include <dpn/input/util.hpp>
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

        onto::Nodes title_nodes;
        //Add a dummy Title that will receive the first lines before an actual Title is encountered
        title_nodes.emplace_back(onto::Type::Title);
        onto::Node *title_ptr = &title_nodes.back();

        onto::Node *code_block_ptr = nullptr;

        unsigned int empty_count = 0u;
        bool is_code_block = false;

        std::vector<metadata::Item> metadata_items;

        for (auto &raw_line: lines)
        {
            gubg::Strange strange{raw_line};

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
            if (strange.empty())
            {
                ++empty_count;
            }
            else
            {
                MSS(process_empty_count());

                if (strange.pop_if("```"))
                {
                    if (!code_block_ptr)
                    {
                        MSS(!!title_ptr, log::internal_error() << "title_ptr should never be nullptr" << std::endl);
                        title_ptr->childs.emplace_back(onto::Type::CodeBlock);
                        code_block_ptr = &title_ptr->childs.back();
                        code_block_ptr->depth = 0;
                    }
                    else
                    {
                        code_block_ptr = nullptr;
                    }
                }
                else
                {
                    if (code_block_ptr)
                    {
                        auto &text = code_block_ptr->text;
                        text += raw_line;
                        text += "\n";
                    }
                    else if (unsigned int depth = 0; util::pop_title(strange, depth))
                    {
                        //We found a Title
                        title_nodes.emplace_back(onto::Type::Title);
                        title_ptr = &title_nodes.back();
                        node_ptr = title_ptr;
                        node_ptr->depth = depth;
                    }
                    else if (unsigned int depth = 0; util::pop_bullet(strange, depth))
                    {
                        //We found a bullet
                        MSS(!!title_ptr, log::internal_error() << "title_ptr should never be nullptr" << std::endl);
                        title_ptr->childs.emplace_back(onto::Type::Line);
                        node_ptr = &title_ptr->childs.back();
                        node_ptr->depth = depth;
                    }
                    else
                    {
                        MSS(false, log::internal_error() << "Line is not empty, not a title and not a bullet. What is it?" << std::endl);
                    }
                }
            }

            if (node_ptr)
            {
                metadata::split(node_ptr->text, metadata_items, strange);
                node_ptr->metadata.setup(metadata_items, filepath.parent_path());
            }
        }

        MSS(standardize_depths_(title_nodes, true));
        for (auto &title_node: title_nodes)
        {
            onto::Nodes childs;
            std::swap(title_node.childs, childs);

            MSS(standardize_depths_(childs, false));

            MSS(append_according_to_depth_(title_node.childs, childs, 0));
        }

        //Move the initial lines before the first Title to file_node.childs
        {
            MSS(!title_nodes.empty(), log::internal_error() << "`title_nodes` should not be empty" << std::endl);
            file_node.childs.swap(title_nodes.front().childs);
            title_nodes.erase(title_nodes.begin());
        }

        MSS(append_according_to_depth_(file_node.childs, title_nodes, 1));

        MSS_END();
    }

    //Rework the depths to ensure child nodes are exactly one level deeper
    bool standardize_depths_(onto::Nodes &nodes, bool unit_step)
    {
        MSS_BEGIN(bool);

        unsigned int level = 0u;
        for (auto ix = 0u; ix < nodes.size(); ++ix)
        {
            auto &s = nodes[ix];
            /* MSS(s.depth > 0); */

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
                        if (unit_step)
                            log::warning() << "Shifting node `" << s2.text << "` by " << shift_count << " deeper to level " << s2.depth << std::endl;
                    }
                }
                ++level;
            }
        }

        MSS_END();
    }

    bool append_according_to_depth_(onto::Nodes &dst, const onto::Nodes &src, unsigned int smallest_depth)
    {
        MSS_BEGIN(bool);

        //Append nodes from src to dst but nest them according to depth
        std::vector<onto::Nodes *> depth0__nodes = {&dst};
        for (const auto &s: src)
        {
            //Create zero-based depth that we can use to index depth0__nodes
            MSS(s.depth >= smallest_depth);
            const auto depth0 = s.depth-smallest_depth;

            if (depth0 == depth0__nodes.size())
            {
                //The nodes at this depth0 are not present yet

                if (depth0__nodes[depth0-1]->empty())
                {
                    //This can happen if the first Line, even before a Title, is a bullet with depth 1
                    depth0__nodes[depth0-1]->emplace_back(onto::Type::Empty);
                }

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
} }
