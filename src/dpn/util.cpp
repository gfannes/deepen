#include <dpn/util.hpp>
#include <gubg/Strange.hpp>

namespace dpn { namespace util { 

    std::pair<unsigned int, unsigned int> lead_count(char ch0, char ch1, const std::string &line)
    {
        std::pair<unsigned int, unsigned int> pair;

        unsigned int ix = 0u;

        for (; ix < line.size(); ++ix)
            if (line[ix] != ch0)
                break;
        pair.first = ix;

        for (; ix < line.size(); ++ix)
            if (line[ix] != ch1)
                break;
        pair.second = ix-pair.first;

        return pair;
    }

} }
