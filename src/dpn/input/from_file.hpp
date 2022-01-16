#ifndef HEADER_dpn_input_from_file_hpp_ALREADY_INCLUDED
#define HEADER_dpn_input_from_file_hpp_ALREADY_INCLUDED

#include <dpn/onto/Node.hpp>
#include <string>
#include <filesystem>

namespace dpn { namespace input { 

    bool load_from_file(onto::Node &file_node, const std::filesystem::path &filepath);
    bool load_from_string(onto::Node &file_node, const std::string &content, const std::filesystem::path &filepath);

    //Privates
    bool standardize_depths_(onto::Nodes &nodes, bool unit_step);
    bool append_according_to_depth_(onto::Nodes &dst, const onto::Nodes &src, unsigned int smallest_depth, onto::Format format);

} }

#endif
