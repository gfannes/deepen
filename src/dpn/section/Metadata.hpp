#ifndef HEADER_dpn_section_Metadata_hpp_ALREADY_INCLUDED
#define HEADER_dpn_section_Metadata_hpp_ALREADY_INCLUDED

#include <string>
#include <vector>
#include <optional>
#include <ostream>

namespace dpn { namespace section { 

    class Metadata
    {
    public:
        std::vector<std::string> lines;
        std::optional<double> effort;

        void stream(std::ostream &os, unsigned int level) const
        {
            const std::string indent(level*2, ' ');

            os << indent << "[Metadata]";
            if (effort)
                os << "(effort:" << *effort << ")";
            if (!lines.empty())
            {
                os << "{" << std::endl;
                for (const auto &line: lines)
                    os << "  " << line << std::endl;
                os << indent << "}" << std::endl;
            }
            else
                os << std::endl;
        }

    private:
    };

    inline std::ostream &operator<<(std::ostream &os, const Metadata &metadata)
    {
        metadata.stream(os, 0);
        return os;
    }

} }

#endif
