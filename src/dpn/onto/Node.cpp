#include <dpn/onto/Node.hpp>

#include <termcolor/termcolor.hpp>

#include <sstream>

namespace dpn { namespace onto { 

    std::size_t Node::child_type_count(Type wanted_type) const
    {
        std::size_t count = (type == wanted_type);
        for (const auto &child: childs)
            count += child.child_type_count(wanted_type);
        return count;
    }

    void Node::set_format(Format fmt)
    {
        format = fmt;
        for (auto &child: childs)
            child.set_format(fmt);
    }

    void Node::aggregate_metadata(const Node *parent, const metadata::Ns__Values &ns__values)
    {
        metadata.aggregate_from_parent(parent ? &parent->metadata : nullptr, ns__values);

        //Depth-first search
        for (auto &child: childs)
        {
            child.aggregate_metadata(this, ns__values);

            metadata.aggregate_from_child(child.metadata);
        }
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

        auto get_agg_local = [&](const auto &filepath) -> const metadata::AggregatedDown *
        {
            const auto p = abs_filepath__node.find(filepath);
            if (p == abs_filepath__node.end())
                return nullptr;
            return &p->second.metadata.agg_down_local;
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
                            switch (format)
                            {
                                case Format::Markdown: os << std::string(depth, '#') << ' '; break;
                                case Format::JIRA: os << 'h' << depth << ". "; break;
                            }
                            break;
                        case Type::Line:
                            if (depth > 0)
                            {
                                switch (format)
                                {
                                    case Format::Markdown: os << std::string(2*(depth-1), ' ') << "* "; break;
                                    case Format::JIRA:     os << std::string(depth, '*') << (depth ? " " : ""); break;
                                }
                                
                            }
                            break;
                        case Type::CodeBlock:
                            switch (format)
                            {
                                case Format::Markdown: os << "```" << std::endl << text << "```" << std::endl; break;
                                case Format::JIRA: os << "{code}" << std::endl << text << "{code}" << std::endl; break;
                            }
                            do_stream = false;
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

                        if (format == Format::JIRA && metadata.input.status)
                        {
                            const auto &status = *metadata.input.status;
                            if (status.state == metadata::State::Blocked)
                            {
                                stream("", "(x)");
                            }
                            else if (status == metadata::Status{metadata::Flow::Validation, true})
                            {
                                stream("", "(/)");
                            }
                        }

                        if (!text.empty())
                            stream("", text);

                        if (metadata.link)
                            stream("", *metadata.link);

                        std::set<metadata::Item> metadata_items;
                        //Update the aggregated metadata items into a new std::set
                        {
                            if (stream_config.include_aggregates)
                            {
                                const auto &agg = metadata.agg_down_global;
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
                        for (const auto &[ns,value]: metadata.input.ns__value)
                        {
                            stream("@", ns);
                            os << ':' << value;
                        }
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
            case StreamConfig::List:
                {
                    const auto is_list = (stream_config.mode == StreamConfig::List);

                    if (stream_config.filter)
                    {
                        const auto filter = *stream_config.filter;
                        const auto p = metadata.agg_down_global.ns__values.find(filter.key);
                        if (p == metadata.agg_down_global.ns__values.end())
                            return;
                        if (!p->second.count(filter.value))
                            return;
                    }

                    const auto is_cancelled = metadata.agg_up.status.state == metadata::State::Cancelled;
                    const auto is_blocked = metadata.agg_up.status.state == metadata::State::Blocked;
                    const auto do_show = [&](){
                        switch (stream_config.mode)
                        {
                            case StreamConfig::Export: return true; break;

                            case StreamConfig::List:
                            if (!is_blocked && metadata.agg_down_global.total_todo().minutes == 0)
                                return false;
                            switch (type)
                            {
                                case Type::Title: break;

                                case Type::Line:
                                if (!stream_config.detailed)
                                    return false;
                                if (depth == 0 && text.empty())
                                    return false;
                                break;

                                default: return false; break;
                            }
                            return true;
                            break;

                            default: return false; break;
                        }
                    }();

                    auto stream_metadata_for_list = [&](bool aggregate){
                        if (is_list)
                        {
                            metadata::Duration todo, effort;
                            if (aggregate)
                            {
                                const auto &agg = metadata.agg_down_global;
                                effort = agg.total_effort;
                                todo = agg.total_todo();
                            }
                            else
                            {
                                const auto &agg = metadata.agg_up;
                                effort = agg.my_effort;
                                todo = agg.my_todo();
                            }

                            if (effort.minutes == 0)
                                os << "     \t     \t";
                            else
                            {
                                if (is_cancelled)
                                    os << "  ~  ";
                                else if (is_blocked)
                                    os << "  X  ";
                                else if (todo.minutes == 0)
                                    os << "  V  ";
                                else
                                    os << todo;
                                os << "\t" << effort << "\t";
                            }

                            if (aggregate)
                            {
                                const auto &agg = metadata.agg_down_global;
                                os << agg.pct_done() << "%\t";
                            }
                            else
                            {
                                os << "    \t";
                            }
                        }
                    };
                    auto stream_metadata_for_export = [&](){
                        if (stream_config.mode == StreamConfig::Export && metadata.agg_down_global.total_effort.minutes > 0)
                        {
                            const auto &agg = metadata.agg_down_global;
                            os << " (" << agg.total_todo() << "/" << agg.total_effort << ", " << agg.pct_done() << '%';
                            if (is_cancelled) os << ", cancelled";
                            if (is_blocked) os << ", blocked";
                            os << ')';
                        }
                    };
                    auto stream_colored = [&](const std::string &str){
                        const auto use_colors = (stream_config.mode == StreamConfig::List);
                        if (!use_colors)
                        {
                            os << str;
                            return;
                        }
                        os << termcolor::colorize;
                        if (is_blocked)
                        {
                            os << termcolor::red;
                        }
                        else
                        {
                            switch (type)
                            {
                                case Type::Title:
                                if (child_type_count(Type::Title) == 1 && !metadata.input.linkpath_abs)
                                    os << termcolor::green;
                                else
                                    switch (stream_config.title_depth_offset+depth)
                                    {
                                        case 1: break;
                                        case 2: os << termcolor::yellow; break;
                                        // case 3: os << termcolor::yellow; break;
                                        // case 4: os << termcolor::magenta; break;
                                        default: os << termcolor::blue; break;
                                    }
                                break;

                                default: os << termcolor::yellow; break;
                            }
                        }
                        os << str << termcolor::reset;
                    };

                    if (do_show)
                    {
                        switch (type)
                        {
                            case Type::Title:
                            stream_metadata_for_list(true);
                            switch (format)
                            {
                                case Format::Markdown: os << std::string(stream_config.title_depth_offset+depth, '#') << ' '; break;
                                case Format::JIRA:     os << 'h' << stream_config.title_depth_offset+depth << ". "; break;
                            }
                            {
                                if (is_blocked) os << "(x) ";
                                if (is_cancelled) os << "~~";
                                stream_colored(text);
                                if (is_cancelled) os << "~~";
                            }
                            if (metadata.link)
                            {
                                os << " => "; stream_colored(metadata.link->key);
                            }
                            stream_metadata_for_export();
                            os << std::endl;
                            break;

                            case Type::Line:
                            stream_metadata_for_list(false);
                            if (depth > 0)
                            {
                                switch (format)
                                {
                                    case Format::Markdown: os << std::string(2*(depth-1), ' ') << "* "; break;
                                    case Format::JIRA:     os << std::string(depth, '*') << (depth ? " " : ""); break;
                                }
                            }
                            stream_colored(text);
                            os << std::endl;
                            break;

                            case Type::CodeBlock:
                            switch (format)
                            {
                                case Format::Markdown: os << "```"    << std::endl; stream_colored(text); os << "```" << std::endl; break;
                                case Format::JIRA:     os << "{code}" << std::endl; stream_colored(text); os << "{code}" << std::endl; break;
                            }
                            break;

                            default: break;
                        }
                    }

                    if (!is_cancelled && !is_blocked)
                    {
                        //Continue streaming recursively

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
