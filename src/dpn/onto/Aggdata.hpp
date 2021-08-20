#ifndef HEADER_dpn_onto_Adddata_hpp_ALREADY_INCLUDED
#define HEADER_dpn_onto_Adddata_hpp_ALREADY_INCLUDED

#include <ostream>
#include <string>

namespace dpn { namespace onto { 

    class Aggdata
    {
    public:
        double effort = 0.0;

        void stream(std::ostream &os, unsigned int level) const
        {
            const std::string indent(level*2, ' ');

            os << indent << "[Aggdata]";
            os << "(effort:" << effort << ")";
            os << std::endl;
        }

    private:
    };

    inline std::ostream &operator<<(std::ostream &os, const Aggdata &aggdata)
    {
        aggdata.stream(os, 0);
        return os;
    }

} }

#endif
