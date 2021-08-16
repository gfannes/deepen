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
        MSS_BEGIN(bool, "");

        std::vector<std::string> lines;
        gubg::string_algo::split_lines(lines, content);

        section::Section *section_ptr = nullptr;
        section::Metadata *metadata_ptr = nullptr;
        unsigned int empty_count = 0u;

        section::Sections tmp_sections;
        section::Metadata metadata;

        for (auto &line: lines)
        {
            if (line.starts_with("---"))
            {
                //Switch from reading Section data to reading Metadata
                metadata_ptr = &metadata;
                section_ptr = nullptr;
                continue;
            }

            auto add_line_to = [&](auto &dst){
                if (line.empty())
                {
                    if (!dst.empty())
                        //Only when there is already some content will we increment empty_count
                        empty_count++;
                }
                else
                {
                    //Add the collected empty lines
                    for (auto ix = 0u; ix < empty_count; ++ix)
                        dst.emplace_back();
                    empty_count = 0u;

                    //Add the actual line
                    dst.emplace_back();
                    std::swap(line, dst.back());
                }
            };

            if (metadata_ptr)
            {
                add_line_to(metadata_ptr->lines);
            }
            else if (const auto pair = util::lead_count('#', ' ', line); pair.first > 0)
            {
                //We found a title
                tmp_sections.emplace_back();
                section_ptr = &tmp_sections.back();
                section_ptr->title_depth = pair.first;
                section_ptr->title = line.substr(pair.first+pair.second);
                empty_count = 0u;
            }
            else if (const auto md = util::parse_metadata(line); !!md)
            {
                //We found metadata

                MSS(!!section_ptr);
                auto &metadata = section_ptr->metadata;

                const auto &key = md->first;
                const auto &value = md->second;
                if (false) {}
                else if (key == "effort")
                {
                    metadata.effort = std::stod(value);
                }
            }
            else
            {
                if (!section_ptr)
                {
                    log::warning() << "Dropping line `" << line << "`, there is no Section to add it to" << std::endl;
                    continue;
                }

                add_line_to(section_ptr->lines);
            }
        }

        MSS(standardize_title_depths_(tmp_sections));

        //Append sections from tmp_sections to sections
        //but nest them according to title_depth
        std::vector<section::Sections *> depth0__sections = {&sections};
        for (const auto &s: tmp_sections)
        {
            //Create zero-based title_depth that we can use to index depth0__sections
            MSS(s.title_depth > 0);
            const auto depth0 = s.title_depth-1;

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

        MSS_END();
    }

    //Rework the title depths to ensure child sections are exactly one level deeper
    bool standardize_title_depths_(section::Sections &sections)
    {
        MSS_BEGIN(bool);

        unsigned int level = 0u;
        for (auto ix = 0u; ix < sections.size(); ++ix)
        {
            auto &s = sections[ix];
            MSS(s.title_depth > 0);

            if (s.title_depth < level)
            {
                //This section is more towards the root: OK
            }
            else if (s.title_depth == level)
            {
                //This section is at the same level: this is a child
            }
            else if (s.title_depth == level+1)
            {
                //This section is one level deeper: this is a child
                ++level;
            }
            else if (s.title_depth > level+1)
            {
                //We only accept a unit step when going deeper
                //We will move all affected sections to the root
                const unsigned int shift_count = s.title_depth-level-1;
                for (auto ix2 = ix; ix2 < sections.size(); ++ix2)
                {
                    auto &s2 = sections[ix2];
                    if (s2.title_depth >= s.title_depth)
                    {
                        s2.title_depth -= shift_count;
                        log::warning() << "Shifting section " << s2.title << " by " << shift_count << " deeper to level " << s2.title_depth << std::endl;
                    }
                }
                ++level;
            }
        }

        MSS_END();
    }

} }
