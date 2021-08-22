#include <dpn/metadata/Metadata.hpp>

namespace dpn { namespace metadata { 

    void Metadata::setup(const std::vector<Item> &items)
    {
        clear();

        Duration effort;
        Status status;
        for (const auto &item: items)
        {
            if (item.is_link)
            {
                link = item;
                input.linkpath = item.value;
            }
            else if (item.key.empty())
            {
                gubg::Strange strange{item.value};
                if (false) {}
                else if (effort.parse(strange))
                    input.effort = effort;
                else if (status.parse(strange))
                    input.status = status;
                else
                    this->items.insert(item);
            }
            else
                this->items.insert(item);
        }
    }

    void Metadata::setup_aggregated()
    {
        if (input.effort)
            agg_local.total_effort = *input.effort;
        if (input.linkpath)
            linkpaths.insert(*input.linkpath);
    }
    void Metadata::aggregate_from_parent(const Metadata &parent)
    {
        //Take status from input, if present, or from agg_local parent status
        agg_local.status = (input.status ? *input.status : parent.agg_local.status);
        agg_local.minimal_status = agg_local.status;

        agg_local.total_done.minutes = agg_local.total_effort.minutes*agg_local.status.fraction_done();
    }
    void Metadata::aggregate_from_child(const Metadata &child)
    {
        agg_local.total_effort += child.agg_local.total_effort;
        agg_local.total_done += child.agg_local.total_done;
        agg_local.minimal_status = Status::minimum(agg_local.minimal_status, child.agg_local.minimal_status);
    }
    void Metadata::finalize_aggregated()
    {
    }

    void Metadata::stream(std::ostream &os, unsigned int level) const
    {
        const std::string indent(level*2, ' ');

        if (!items.empty())
        {
            for (const auto &item: items)
                os << indent << "[metadata::Item](key:" << item.key << ")(value:" << item.value << ")" << std::endl;
        }

        if (input.effort || input.status || input.linkpath)
        {
            os << indent << "[metadata::Input]";
            if (input.effort)
                os << "(effort:" << *input.effort << ")";
            if (input.status)
                os << "(status:" << *input.status << ")";
            if (input.linkpath)
                os << "(linkpath:" << *input.linkpath << ")";
            os << std::endl;
        }

        os << indent << "[Links]{" << std::endl;
        for (const auto &linkpath: linkpaths)
            os << indent << "  " << "[Path](" << linkpath << ")" << std::endl;
        os << indent << "}" << std::endl;

        auto stream_agg = [&](const char *name, const auto &agg){
            os << indent << "[" << name << "]";
            os << "(total_effort:" << agg.total_effort << ")";
            os << "(total_done:" << agg.total_done << ")";
            os << "(status:" << agg.status << ")";
            os << "(minimal_status:" << agg.minimal_status << ")";
            os << std::endl;
        };
        stream_agg("agg_local", agg_local);
        stream_agg("agg_global", agg_global);
    }

} }
