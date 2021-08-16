#ifndef HEADER_dpn_util_hpp_ALREADY_INCLUDED
#define HEADER_dpn_util_hpp_ALREADY_INCLUDED

#include <string>
#include <utility>
#include <optional>

namespace dpn { namespace util { 

    //Return how many times ch0 and ch1 occur at the start of `line`
    std::pair<unsigned int, unsigned int> lead_count(char ch0, char ch1, const std::string &line);

    using KeyValue = std::pair<std::string, std::string>;
    std::optional<KeyValue> parse_metadata(const std::string &line);

} }

#endif
