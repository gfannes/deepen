#ifndef HEADER_dpn_util_hpp_ALREADY_INCLUDED
#define HEADER_dpn_util_hpp_ALREADY_INCLUDED

#include <string>
#include <utility>

namespace dpn { namespace util { 

    //Return how many times ch0 and ch1 occur at the start of `line`
    std::pair<unsigned int, unsigned int> lead_count(char ch0, char ch1, const std::string &line);

} }

#endif
