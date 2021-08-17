#ifndef HEADER_dpn_section_Section_hpp_ALREADY_INCLUDED
#define HEADER_dpn_section_Section_hpp_ALREADY_INCLUDED

#include <dpn/section/enums.hpp>
#include <dpn/metadata/Metadata.hpp>
#include <string>
#include <vector>
#include <optional>
#include <ostream>

namespace dpn { namespace section { 

    class Section
    {
    public:
        Type type = Type::Line;

        std::string text;
        unsigned int depth = 0;

        std::vector<Section> childs;
        std::optional<std::string> filepath;
        metadata::Metadata metadata;

        Section() {}
        Section(Type type): type(type) {}

        void aggregate_metadata(const Section *parent);

        struct StreamConfig
        {
            enum Mode {Original, Export, Naft};
            Mode mode = Naft;
        };
        void stream(std::ostream &os, unsigned int level, const StreamConfig &stream_config) const;

        bool operator==(const Section &rhs) const;
        bool operator!=(const Section &rhs) const {return !operator==(rhs);}

    private:
    };

    inline std::ostream &operator<<(std::ostream &os, const Section &section)
    {
        Section::StreamConfig stream_config;
        section.stream(os, 0u, stream_config);
        return os;
    }

    using Sections = std::vector<Section>;

} }

#endif
