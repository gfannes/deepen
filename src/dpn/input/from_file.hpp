#ifndef HEADER_dpn_input_from_file_hpp_ALREADY_INCLUDED
#define HEADER_dpn_input_from_file_hpp_ALREADY_INCLUDED

#include <dpn/onto/Node.hpp>
#include <string>

namespace dpn { namespace input { 

    bool append_from_file(onto::Nodes &nodes, const std::string &filepath);
    bool append_from_string(onto::Nodes &nodes, const std::string &content, const std::string &filepath);

    //Privates
    bool standardize_depths_(onto::Nodes &nodes);

} }

#endif
