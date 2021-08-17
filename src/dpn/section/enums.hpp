#ifndef HEADER_dpn_section_enums_hpp_ALREADY_INCLUDED
#define HEADER_dpn_section_enums_hpp_ALREADY_INCLUDED

#include <ostream>

namespace dpn { namespace section { 

    enum class Type
    {
        Title, Line,
    };
    inline std::ostream &operator<<(std::ostream &os, Type type)
    {
        switch (type)
        {
            case Type::Title: os << "Title"; break;
            case Type::Line: os << "Line"; break;
            default: os << "<unknown Type>"; break;
        }
        return os;
    }

} }

#endif
