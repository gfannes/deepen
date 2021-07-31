#ifndef HEADER_dpn_input_from_file_hpp_ALREADY_INCLUDED
#define HEADER_dpn_input_from_file_hpp_ALREADY_INCLUDED

#include <dpn/section/Section.hpp>
#include <string>

namespace dpn { namespace input { 

    bool append_from_file(section::Sections &sections, const std::string &filepath);
    bool append_from_string(section::Sections &sections, const std::string &content, const std::string &filepath);

    //Privates
    bool standardize_title_depths_(section::Sections &sections);

} }

#endif
