#ifndef HEADER_dpn_read_markdown_ALREADY_INCLUDED
#define HEADER_dpn_read_markdown_ALREADY_INCLUDED

#include <dpn/Node.hpp>

#include <string>

namespace dpn { namespace read { 

	bool markdown(Nodes &, const std::string &);
	bool freemind(Nodes &, const std::string &);

} } 

#endif