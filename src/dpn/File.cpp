#include <dpn/File.hpp>
#include <dpn/log.hpp>
#include <gubg/mss.hpp>
#include <gubg/file/system.hpp>
#include <gubg/string_algo/algo.hpp>

namespace dpn { 

    bool File::load_from_string(const std::string &str)
    {
        MSS_BEGIN(bool);

        std::vector<std::string> lines;
        gubg::string_algo::split_lines(lines, str);

        section::Section *section_ptr = nullptr;
        section::Metadata *metadata_ptr = nullptr;
        unsigned int empty_count = 0u;

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
            else if (line.starts_with("#"))
            {
                //We found a title
                if (!section_ptr)
                {
                    section_ptr = &main_section;
                }
                else
                {
                    sub_sections.emplace_back();
                    section_ptr = &sub_sections.back();
                }
                section_ptr->title = line;
                empty_count = 0u;
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

        MSS_END();
    }
    bool File::load_from_file(const std::string &filepath)
    {
        MSS_BEGIN(bool);
        std::string str;
        MSS(gubg::file::read(str, filepath));
        MSS(load_from_string(str));
        MSS_END();
    }

    void File::stream(std::ostream &os) const
    {
        os << main_section;
        for (const auto &sub_section: sub_sections)
            os << sub_section;
        os << metadata;
    }
}
