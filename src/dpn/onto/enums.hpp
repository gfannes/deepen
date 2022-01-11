#ifndef HEADER_dpn_onto_enums_hpp_ALREADY_INCLUDED
#define HEADER_dpn_onto_enums_hpp_ALREADY_INCLUDED

#include <ostream>

namespace dpn { namespace onto { 

    enum class Type
    {
        Root, Link, File, Title, Line, CodeBlock, Empty,
    };

    inline std::ostream &operator<<(std::ostream &os, Type type)
    {
        switch (type)
        {
            case Type::Root:      os << "Root";  break;
            case Type::Link:      os << "Link";  break;
            case Type::File:      os << "File";  break;
            case Type::Title:     os << "Title"; break;
            case Type::Line:      os << "Line";  break;
            case Type::CodeBlock:  os << "CodeBlock";  break;
            case Type::Empty:     os << "Empty"; break;
            default: os << "<unknown Type>"; break;
        }
        return os;
    }

} }

#endif
