#include <dpn/onto/Node.hpp>
#include <sstream>

namespace dpn { namespace onto { 

    void Node::aggregate_metadata(const Node *parent)
    {
        metadata.setup_aggregated();

        if (parent)
            metadata.aggregate_from_parent(parent->metadata);

        //Depth-first search
        for (auto &child: childs)
        {
            child.aggregate_metadata(this);

            metadata.aggregate_from_child(child.metadata);
        }

        metadata.finalize_aggregated();
    }

    void Node::stream(std::ostream &os, unsigned int level, const StreamConfig &stream_config) const
    {
        switch (stream_config.mode)
        {
            case StreamConfig::Original:
                {
                    switch (type)
                    {
                        case Type::Title: os << std::string(depth, '#') << ' '; break;
                        case Type::Line: break;
                    }
                    os << text;

                    std::set<metadata::Item> metadata_items;
                    //Update the aggregated metadata items into a new std::set
                    {
                        if (metadata.aggregated_opt && type == Type::Title)
                        {
                            const auto &agg = *metadata.aggregated_opt;
                            std::ostringstream oss;

                            oss.str(""); oss << agg.pct_done() << '%';
                            metadata_items.insert(metadata::Item("C", oss.str()));

                            oss.str(""); oss << agg.total_effort;
                            metadata_items.insert(metadata::Item("E", oss.str()));

                            oss.str(""); oss << agg.minimal_status;
                            metadata_items.insert(metadata::Item("S", oss.str()));

                            oss.str(""); oss << agg.total_todo;
                            metadata_items.insert(metadata::Item("T", oss.str()));
                        }
                        for (const auto &item: metadata.items)
                        {
                            if (false) {}
                            else if (item.key == "C") {}
                            else if (item.key == "E") {}
                            else if (item.key == "S") {}
                            else if (item.key == "T") {}
                            else metadata_items.insert(item);
                        }
                    }

                    for (const auto &item: metadata_items)
                        os << ' ' << item;
                    if (metadata.input_opt)
                    {
                        const auto &input = *metadata.input_opt;
                        if (input.effort.minutes > 0.0)
                            os << " @" << input.effort;
                        if (input.status != metadata::Status{metadata::State::Requirement, false})
                            os << " @" << input.status;
                    }

                    os << std::endl;

                    for (const auto &child: childs)
                        child.stream(os, level+1, stream_config);
                }
                break;

            case StreamConfig::Export:
                switch (type)
                {
                    case Type::Title:
                        os << std::string(depth, '#') << ' ' << text;
                        if (metadata.aggregated_opt)
                        {
                            const auto &agg = *metadata.aggregated_opt;
                            os << " (" << agg.total_todo << "/" << agg.total_effort << ", " << agg.pct_done() << "%)";
                        }
                        os << std::endl;
                        break;
                    case Type::Line:
                        os << text << std::endl;
                        break;
                }
                for (const auto &child: childs)
                    child.stream(os, level+1, stream_config);
                break;

            case StreamConfig::Naft:
                {
                    const std::string indent(level*2, ' ');

                    os << indent << "[Node](type:" << type << ")(text:" << text << ")(depth:" << depth << ")";
                    if (filepath)
                        os << "(filepath:" << *filepath << ")";
                    os << "{" << std::endl;
                    metadata.stream(os, level+1);
                    for (const auto &child: childs)
                        child.stream(os, level+1, stream_config);
                    os << indent << "}" << std::endl;
                }
                break;
        }
    }

    bool Node::operator==(const Node &rhs) const
    {
        if (text != rhs.text) return false;
        if (childs.size() != rhs.childs.size()) return false;
        for (auto ix = 0u; ix < childs.size(); ++ix)
            if (childs[ix] != rhs.childs[ix]) return false;
        if (!!filepath != !!rhs.filepath) return false;
        if (!!filepath && *filepath != *rhs.filepath) return false;
        return true;
    }

} }
