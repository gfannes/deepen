#ifndef HEADER_dpn_onto_Node_hpp_ALREADY_INCLUDED
#define HEADER_dpn_onto_Node_hpp_ALREADY_INCLUDED

#include <dpn/onto/enums.hpp>
#include <dpn/metadata/Metadata.hpp>
#include <string>
#include <vector>
#include <optional>
#include <ostream>

namespace dpn { namespace onto { 

    class Node
    {
    public:
        Type type = Type::Line;

        std::string text;
        unsigned int depth = 0;

        std::vector<Node> childs;
        std::optional<std::string> filepath;
        metadata::Metadata metadata;

        Node() {}
        Node(Type type): type(type) {}

        void aggregate_metadata(const Node *parent);

        struct StreamConfig
        {
            enum Mode {Original, Export, Naft};
            Mode mode = Naft;
        };
        void stream(std::ostream &os, unsigned int level, const StreamConfig &stream_config) const;

        bool operator==(const Node &rhs) const;
        bool operator!=(const Node &rhs) const {return !operator==(rhs);}

    private:
    };

    inline std::ostream &operator<<(std::ostream &os, const Node &node)
    {
        Node::StreamConfig stream_config;
        node.stream(os, 0u, stream_config);
        return os;
    }

    using Nodes = std::vector<Node>;

} }

#endif
