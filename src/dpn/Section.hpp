#ifndef HEADER_dpn_Section_hpp_ALREADY_INCLUDED
#define HEADER_dpn_Section_hpp_ALREADY_INCLUDED

#include <string>
#include <vector>
#include <ostream>

namespace dpn { 

    class Section
    {
    public:
        std::string title;
        std::vector<std::string> body;

        void stream(std::ostream &os) const
        {
            os << "[Section](title:" << title << "){" << std::endl;
            for (const auto &line: body)
                os << "  " << line << std::endl;
            os << "}" << std::endl;
        }

    private:
    };

    inline std::ostream &operator<<(std::ostream &os, const Section &section)
    {
        section.stream(os);
        return os;
    }

}

#endif
