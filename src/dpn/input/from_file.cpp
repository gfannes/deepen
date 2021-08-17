#include <dpn/input/from_file.hpp>
#include <dpn/util.hpp>
#include <dpn/log.hpp>
#include <gubg/file/system.hpp>
#include <gubg/string_algo/algo.hpp>
#include <gubg/Strange.hpp>
#include <gubg/mss.hpp>
#include <cassert>

namespace dpn { namespace input { 

    bool append_from_file(section::Sections &sections, const std::string &filepath)
    {
        MSS_BEGIN(bool);

        std::string content;
        MSS(gubg::file::read(content, filepath));

        MSS(append_from_string(sections, content, filepath));

        MSS_END();
    }

    bool append_from_string(section::Sections &sections, const std::string &content, const std::string &filepath)
    {
        MSS_BEGIN(bool);

        std::vector<std::string> lines;
        gubg::string_algo::split_lines(lines, content);

        section::Section *title_ptr = nullptr;
        section::Section *section_ptr = nullptr;
        metadata::Metadata *metadata_ptr = nullptr;
        unsigned int empty_count = 0u;

        section::Sections title_sections;
        metadata::Metadata metadata;

        std::string line;

        std::vector<metadata::Item> metadata_items;

        for (auto &raw_line: lines)
        {
            if (raw_line.starts_with("---"))
            {
                //Switch from reading Section data to reading Metadata
                metadata_ptr = &metadata;
                title_ptr = nullptr;
                continue;
            }

            auto process_empty_count = [&](){
                if (title_ptr)
                {
                    //Add the collected empty lines
                    for (auto ix = 0u; ix < empty_count; ++ix)
                        title_ptr->childs.emplace_back(section::Type::Line);
                    empty_count = 0u;
                }
            };

            if (metadata_ptr)
            {
                section_ptr = nullptr;
            }
            else if (const auto pair = util::lead_count('#', ' ', raw_line); pair.first > 0)
            {
                //We found a title

                process_empty_count();

                title_sections.emplace_back(section::Type::Title);
                title_ptr = &title_sections.back();
                section_ptr = title_ptr;
                title_ptr->depth = pair.first;
                line = raw_line.substr(pair.first+pair.second);
            }
            else
            {
                if (!title_ptr)
                {
                    log::warning() << "Dropping raw_line `" << raw_line << "`, there is no Section to add it to" << std::endl;
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
                    title_ptr->childs.emplace_back(section::Type::Line);
                    section_ptr = &title_ptr->childs.back();
                    line = raw_line;
                }
            }

            if (section_ptr)
            {
                metadata::split(section_ptr->text, metadata_items, line);
                section_ptr->metadata.setup(metadata_items);
            }
        }

        MSS(standardize_depths_(title_sections));

        //Append sections from title_sections to sections
        //but nest them according to depth
        std::vector<section::Sections *> depth0__sections = {&sections};
        for (const auto &s: title_sections)
        {
            //Create zero-based depth that we can use to index depth0__sections
            MSS(s.depth > 0);
            const auto depth0 = s.depth-1;

            if (depth0 == depth0__sections.size())
            {
                //The sections at this depth0 is not present yet
                MSS(!depth0__sections[depth0-1]->empty());
                auto &parent = depth0__sections[depth0-1]->back();
                depth0__sections.push_back(&parent.childs);
            }
            MSS(depth0 < depth0__sections.size());

            depth0__sections.resize(depth0+1);
            depth0__sections[depth0]->push_back(s);

            if (depth0 == 0)
                depth0__sections[depth0]->back().filepath = filepath;
        }

        //Aggregate metadata for all root sections
        for (auto &section: sections)
            section.aggregate_metadata(nullptr);

        MSS_END();
    }

    //Rework the text depths to ensure child sections are exactly one level deeper
    bool standardize_depths_(section::Sections &sections)
    {
        MSS_BEGIN(bool);

        unsigned int level = 0u;
        for (auto ix = 0u; ix < sections.size(); ++ix)
        {
            auto &s = sections[ix];
            MSS(s.depth > 0);

            if (s.depth < level)
            {
                //This section is more towards the root: OK
            }
            else if (s.depth == level)
            {
                //This section is at the same level: this is a child
            }
            else if (s.depth == level+1)
            {
                //This section is one level deeper: this is a child
                ++level;
            }
            else if (s.depth > level+1)
            {
                //We only accept a unit step when going deeper
                //We will move all affected sections to the root
                const unsigned int shift_count = s.depth-level-1;
                for (auto ix2 = ix; ix2 < sections.size(); ++ix2)
                {
                    auto &s2 = sections[ix2];
                    if (s2.depth >= s.depth)
                    {
                        s2.depth -= shift_count;
                        log::warning() << "Shifting section " << s2.text << " by " << shift_count << " deeper to level " << s2.depth << std::endl;
                    }
                }
                ++level;
            }
        }

        MSS_END();
    }

} }
