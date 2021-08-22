#ifndef HEADER_dpn_onto_Node_hpp_ALREADY_INCLUDED
#define HEADER_dpn_onto_Node_hpp_ALREADY_INCLUDED

#include <dpn/onto/enums.hpp>
#include <dpn/metadata/Metadata.hpp>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <ostream>

namespace dpn { namespace onto { 

    class Node
    {
    public:
        Type type = Type::Line;

        std::string text;
        unsigned int depth = 0;
        std::string filepath;//Only used for Type::File

        std::vector<Node> childs;
        metadata::Metadata metadata;

        Node() {}
        Node(Type type): type(type) {}

        void aggregate_metadata(const Node *parent);

        using Filepath__Node = std::map<std::string, onto::Node>;
        bool merge_linkpaths(unsigned int &count, const Filepath__Node &filepath__node);
        bool aggregate_linkpaths(const std::map<std::string, onto::Node> &filepath__node);

        template <typename Ftor>
        void each_linkpath(Ftor &&ftor) const
        {
            if (metadata.input.linkpath)
                ftor(*metadata.input.linkpath);
            for (const auto &child: childs)
                child.each_linkpath(ftor);
        }

        struct StreamConfig
        {
            enum Mode {Original, Export, Naft};
            Mode mode = Naft;
            bool include_aggregates = false;
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
    using Filepath__Node = std::map<std::string, onto::Node>;

} }

#endif
