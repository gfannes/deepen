#ifndef HEADER_dpn_onto_Node_hpp_ALREADY_INCLUDED
#define HEADER_dpn_onto_Node_hpp_ALREADY_INCLUDED

#include <dpn/onto/enums.hpp>
#include <dpn/metadata/Metadata.hpp>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <filesystem>
#include <ostream>

namespace dpn { namespace onto { 

    class Node
    {
    public:
        Type type = Type::Empty;

        std::string text;
        unsigned int depth = 0;
        std::filesystem::path filepath;//Only used for Type::File

        std::vector<Node> childs;
        metadata::Metadata metadata;

        Node() {}
        Node(Type type): type(type) {}

        void aggregate_metadata(const Node *parent);

        using AbsFilepath__Node = std::map<std::filesystem::path, onto::Node>;
        bool merge_linkpaths(unsigned int &count, const AbsFilepath__Node &abs_filepath__node);
        bool aggregate_linkpaths(const AbsFilepath__Node &abs_filepath__node);

        template <typename Ftor>
        void each_abs_linkpath(Ftor &&ftor) const
        {
            if (metadata.input.linkpath_abs)
                ftor(*metadata.input.linkpath_abs);
            for (const auto &child: childs)
                child.each_abs_linkpath(ftor);
        }

        struct StreamConfig
        {
            enum Mode {Original, Export, Naft};
            Mode mode = Naft;
            bool include_aggregates = false;
            const AbsFilepath__Node *abs_filepath__node = nullptr;
            unsigned int title_depth_offset = 0;
            std::optional<metadata::Item> filter;
        };
        void stream(std::ostream &os, unsigned int level, const StreamConfig &stream_config) const;

    private:
    };

    inline std::ostream &operator<<(std::ostream &os, const Node &node)
    {
        Node::StreamConfig stream_config;
        node.stream(os, 0u, stream_config);
        return os;
    }

    using Nodes = std::vector<Node>;
    using AbsFilepath__Node = std::map<std::filesystem::path, onto::Node>;

} }

#endif
