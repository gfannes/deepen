#ifndef HEADER_dpn_section_Section_hpp_ALREADY_INCLUDED
#define HEADER_dpn_section_Section_hpp_ALREADY_INCLUDED

#include <dpn/section/Metadata.hpp>
#include <string>
#include <vector>
#include <optional>
#include <ostream>

namespace dpn { namespace section { 

    class Section
    {
    public:
        std::string title;
        unsigned int title_depth = 0;
        std::vector<std::string> lines;
        std::vector<Section> childs;
        std::optional<std::string> filepath;

        void stream(std::ostream &os, unsigned int level) const
        {
            const std::string indent(level*2, ' ');

            os << indent << "[Section](title:" << title << ")(title_depth:" << title_depth << ")";
            if (filepath)
                os << "(filepath:" << *filepath << ")";
            os << "{" << std::endl;
            for (const auto &line: lines)
                os << indent << "  " << line << std::endl;
            for (const auto &child: childs)
                child.stream(os, level+1);
            os << indent << "}" << std::endl;
        }

        bool operator==(const Section &rhs) const
        {
            if (title != rhs.title) return false;
            if (lines != rhs.lines) return false;
            if (childs.size() != rhs.childs.size()) return false;
            for (auto ix = 0u; ix < childs.size(); ++ix)
                if (childs[ix] != rhs.childs[ix]) return false;
            if (!!filepath != !!rhs.filepath) return false;
            if (!!filepath && *filepath != *rhs.filepath) return false;
            return true;
        }
        bool operator!=(const Section &rhs) const {return !operator==(rhs);}

    private:
    };

    inline std::ostream &operator<<(std::ostream &os, const Section &section)
    {
        section.stream(os, 0u);
        return os;
    }

    using Sections = std::vector<Section>;

} }

#endif
