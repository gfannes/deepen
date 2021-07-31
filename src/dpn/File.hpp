#ifndef HEADER_dpn_File_hpp_ALREADY_INCLUDED
#define HEADER_dpn_File_hpp_ALREADY_INCLUDED

#include <dpn/section/Metadata.hpp>
#include <dpn/section/Section.hpp>
#include <string>
#include <vector>
#include <ostream>

namespace dpn { 

    class File
    {
    public:
        section::Metadata metadata;
        section::Section main_section;
        std::vector<section::Section> sub_sections;

        //TODO: Remove these and replace with dpn::input::append_from_file()
        bool load_from_string(const std::string &str);
        bool load_from_file(const std::string &filepath);

        void stream(std::ostream &os) const;

    private:
        std::string content_;
    };

    inline std::ostream &operator<<(std::ostream &os, const File &file)
    {
        file.stream(os);
        return os;
    }

}

#endif
