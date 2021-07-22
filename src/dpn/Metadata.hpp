#ifndef HEADER_dpn_Metadata_hpp_ALREADY_INCLUDED
#define HEADER_dpn_Metadata_hpp_ALREADY_INCLUDED

#include <string>
#include <vector>
#include <ostream>

namespace dpn { 

    class Metadata
    {
    public:
        std::vector<std::string> lines;

        void stream(std::ostream &os) const
        {
            os << "[Metadata]{" << std::endl;
            for (const auto &line: lines)
                os << "  " << line << std::endl;
            os << "}" << std::endl;
        }

    private:
    };

    inline std::ostream &operator<<(std::ostream &os, const Metadata &metadata)
    {
        metadata.stream(os);
        return os;
    }

}

#endif
