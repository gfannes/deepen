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
            case Type::CodeBlock: os << "CodeBlock";  break;
            case Type::Empty:     os << "Empty"; break;
            default: os << "<unknown Type>"; break;
        }
        return os;
    }

    enum class Format
    {
        Markdown, JIRA,
    };

    template <typename Ftor>
    bool parse_format(const std::string &str, Ftor &&ftor)
    {
        if (str == "md" || str == "markdown" || str == "Markdown")
            return (ftor(Format::Markdown), true);
        if (str == "jira" || str == "JIRA" || str == "textile")
            return (ftor(Format::JIRA), true);
        return false;
    }

    inline std::ostream &operator<<(std::ostream &os, Format format)
    {
        switch (format)
        {
            case Format::Markdown:  os << "Markdown";  break;
            case Format::JIRA:      os << "JIRA";  break;
            default: os << "<unknown Format>"; break;
        }
        return os;
    }

} }

#endif
