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

    bool Node::merge_linkpaths(unsigned int &count, const AbsFilepath__Node &abs_filepath__node)
    {
        MSS_BEGIN(bool);

        auto merge = [&](const auto &src){
            const auto count_before = metadata.linkpaths.size();
            metadata.linkpaths.insert(src.begin(), src.end());
            const auto count_after = metadata.linkpaths.size();

            count += count_after - count_before;
        };

        for (auto &child: childs)
        {
            MSS(child.merge_linkpaths(count, abs_filepath__node));

            merge(child.metadata.linkpaths);
        }

        const auto copy_linkpaths = metadata.linkpaths;
        for (const auto &linkpath: copy_linkpaths)
        {
            const auto p = abs_filepath__node.find(linkpath);
            MSS(p != abs_filepath__node.end(), log::internal_error() << "Could not find node `" << linkpath << "`" << std::endl);
            const auto &linknode = p->second;

            merge(linknode.metadata.linkpaths);
        }

        MSS_END();
    }

    bool Node::aggregate_linkpaths(const AbsFilepath__Node &abs_filepath__node)
    {
        MSS_BEGIN(bool);

        auto get_agg_local = [&](const auto &filepath) -> const metadata::Aggregated *
        {
            const auto p = abs_filepath__node.find(filepath);
            if (p == abs_filepath__node.end())
                return nullptr;
            return &p->second.metadata.agg_local;
        };
        MSS(metadata.aggregate_global(get_agg_local));

        for (auto &child: childs)
        {
            MSS(child.aggregate_linkpaths(abs_filepath__node));
        }

        MSS_END();
    }

    void Node::stream(std::ostream &os, unsigned int level, const StreamConfig &stream_config) const
    {
        switch (stream_config.mode)
        {
            case StreamConfig::Original:
                {
                    bool do_stream = true;
                    switch (type)
                    {
                        case Type::Title:
                            os << std::string(depth, '#') << ' ';
                            break;
                        case Type::Line:
                            if (depth > 0)
                                os << std::string(2*(depth-1), ' ') << "* ";
                            break;
                        default: do_stream = false; break;
                    }

                    if (do_stream)
                    {
                        bool add_space = false;
                        auto stream = [&](const char *prefix, const auto &e){
                            if (add_space)
                                os << ' ';
                            add_space = true;
                            os << prefix << e;
                        };

                        if (!text.empty())
                            stream("", text);

                        if (metadata.link)
                            stream("", *metadata.link);

                        std::set<metadata::Item> metadata_items;
                        //Update the aggregated metadata items into a new std::set
                        {
                            if (stream_config.include_aggregates)
                            {
                                const auto &agg = metadata.agg_global;
                                if (type == Type::Title && agg.total_effort.minutes > 0)
                                {
                                    std::ostringstream oss;

                                    oss.str(""); oss << agg.pct_done() << '%';
                                    metadata_items.insert(metadata::Item(metadata::Item::Generated, "C", oss.str()));

                                    oss.str(""); oss << agg.total_effort;
                                    metadata_items.insert(metadata::Item(metadata::Item::Generated, "E", oss.str()));

                                    if (false)
                                    {
                                        oss.str(""); oss << agg.minimal_status;
                                        metadata_items.insert(metadata::Item(metadata::Item::Generated, "S", oss.str()));
                                    }

                                    oss.str(""); oss << agg.total_done;
                                    metadata_items.insert(metadata::Item(metadata::Item::Generated, "D", oss.str()));

                                    oss.str(""); oss << agg.total_todo();
                                    metadata_items.insert(metadata::Item(metadata::Item::Generated, "T", oss.str()));
                                }
                            }
                            for (const auto &item: metadata.items)
                                if (item.type == metadata::Item::User)
                                   metadata_items.insert(item);
                        }

                        for (const auto &item: metadata_items)
                            stream("", item);
                        if (metadata.input.effort)
                            stream("@", *metadata.input.effort);
                        if (metadata.input.status)
                            stream("@", *metadata.input.status);

                        os << std::endl;
                    }

                    for (const auto &child: childs)
                        child.stream(os, level+1, stream_config);
                }
                break;

            case StreamConfig::Export:
                {
                    const auto is_cancelled = metadata.agg_local.status.state == metadata::State::Cancelled;

                    switch (type)
                    {
                        case Type::Title:
                            os << std::string(stream_config.title_depth_offset+depth, '#') << ' ';
                            {
                                if (is_cancelled) os << "~~";
                                os << text;
                                if (is_cancelled) os << "~~";
                            }
                            if (metadata.link)
                                os << ' ' << metadata.link->key;
                            if (metadata.agg_global.total_effort.minutes > 0)
                            {
                                const auto &agg = metadata.agg_global;
                                os << " (" << agg.total_todo() << "/" << agg.total_effort << ", " << agg.pct_done() << '%';
                                if (is_cancelled) os << ", cancelled";
                                os << ')';
                            }
                            os << std::endl;
                            break;
                        case Type::Line:
                            if (depth > 0)
                                os << std::string(2*(depth-1), ' ') << "* ";
                            os << text << std::endl;
                            break;
                    }
                    if (!is_cancelled)
                    {
                        if (metadata.input.linkpath_abs)
                        {
                            if (stream_config.abs_filepath__node)
                            {
                                const auto &abs_filepath__node = *stream_config.abs_filepath__node;
                                const auto it = abs_filepath__node.find(*metadata.input.linkpath_abs);
                                if (it != abs_filepath__node.end())
                                {
                                    const auto &linknode = it->second;
                                    StreamConfig my_stream_config = stream_config;
                                    if (type == Type::Title)
                                        my_stream_config.title_depth_offset += depth;
                                    linknode.stream(os, level+1, my_stream_config);
                                }
                            }
                        }
                        for (const auto &child: childs)
                            child.stream(os, level+1, stream_config);
                    }
                }
                break;

            case StreamConfig::Naft:
                {
                    const std::string indent(level*2, ' ');

                    os << indent << "[Node](type:" << type << ")(text:" << text << ")(depth:" << depth << ")(filepath:" << filepath.string() << "){" << std::endl;
                    metadata.stream(os, level+1);
                    for (const auto &child: childs)
                        child.stream(os, level+1, stream_config);
                    os << indent << "}" << std::endl;
                }
                break;
        }
    }

} }
