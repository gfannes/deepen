#ifndef HEADER_dpn_read_markdown_ALREADY_INCLUDED
#define HEADER_dpn_read_markdown_ALREADY_INCLUDED

#include <dpn/onto/Node.hpp>

#include <string>

namespace dpn { namespace read { 

	bool markdown(onto::Nodes &, const std::string &);
	bool naft(onto::Nodes &, const std::string &);
	bool freemind(onto::Nodes &, const std::string &);

} } 

#endif