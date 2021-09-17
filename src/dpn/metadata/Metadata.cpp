#include <dpn/metadata/Metadata.hpp>

namespace dpn { namespace metadata { 

    void Metadata::setup(const std::vector<Item> &items, const std::filesystem::path &cwd)
    {
        clear();

        Duration effort;
        Status status;
        for (const auto &item: items)
        {
            switch (item.type)
            {
                case Item::User:
                if (item.is_link)
                {
                    link = item;
                    input.linkpath_rel = item.value;
                    input.linkpath_abs = (cwd / item.value).lexically_normal();
                }
                else if (item.key.empty())
                {
                    gubg::Strange strange{item.value};
                    if (effort.parse(strange))
                        input.effort = effort;
                    else if (status.parse(strange))
                        input.status = status;
                    else
                        this->items.insert(item);
                }
                else
                {
                    input.ns__value[item.key] = item.value;
                }
                break;

                default: break;
            }
        }
    }

    void Metadata::setup_aggregated()
    {
        if (input.effort)
        {
            const auto fraction_effort = (input.status ? input.status->fraction_effort() : 1.0);
            agg_local.total_effort.minutes = input.effort->minutes * fraction_effort;
        }
        if (input.linkpath_abs)
            linkpaths.insert(*input.linkpath_abs);

        agg_local.ns__value = input.ns__value;
        for (const auto &[ns,value]: input.ns__value)
            agg_local.ns__values[ns].insert(value);
    }
    void Metadata::aggregate_from_parent(const Metadata &parent)
    {
        //Take status from input, if present, or from agg_local parent status
        agg_local.status = (input.status ? *input.status : parent.agg_local.status);
        agg_local.minimal_status = agg_local.status;

        agg_local.total_done.minutes = agg_local.total_effort.minutes*agg_local.status.fraction_done();

        for (const auto &[ns,value]: parent.agg_local.ns__value)
            if (!agg_local.ns__value.count(ns))
                agg_local.ns__value[ns] = value;
    }
    void Metadata::aggregate_from_child(const Metadata &child)
    {
        agg_local.total_effort += child.agg_local.total_effort;
        agg_local.total_done += child.agg_local.total_done;
        agg_local.minimal_status = Status::minimum(agg_local.minimal_status, child.agg_local.minimal_status);

        for (const auto &[ns,values]: child.agg_local.ns__values)
            agg_local.ns__values[ns].insert(values.begin(), values.end());
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
                os << indent << "[metadata::Item](type:" << (int)item.type << ")(key:" << item.key << ")(value:" << item.value << ")(is_link:" << item.is_link << ")" << std::endl;
        }

        if (input.effort || input.status || input.linkpath_rel)
        {
            os << indent << "[metadata::Input]";
            if (input.effort)
                os << "(effort:" << *input.effort << ")";
            if (input.status)
                os << "(status:" << *input.status << ")";
            if (input.linkpath_rel)
                os << "(linkpath_rel:" << *input.linkpath_rel << ")";
            if (input.linkpath_abs)
                os << "(linkpath_abs:" << input.linkpath_abs->string() << ")";
            if (!input.ns__value.empty())
            {
                os << indent << "{" << std::endl;
                for (const auto &[ns,value]: input.ns__value)
                    os << indent << "  [Ns__Value]" << "(ns:" << ns << ")(value:" << value << ")" << std::endl;
                os <<  indent << "}" << std::endl;
            }
            os << std::endl;
        }

        os << indent << "[Links]{" << std::endl;
        for (const auto &linkpath: linkpaths)
            os << indent << "  " << "[Path](" << linkpath.string() << ")" << std::endl;
        os << indent << "}" << std::endl;

        auto stream_agg = [&](const char *name, const auto &agg){
            os << indent << "[" << name << "]";
            os << "(total_effort:" << agg.total_effort << ")";
            os << "(total_done:" << agg.total_done << ")";
            os << "(status:" << agg.status << ")";
            os << "(minimal_status:" << agg.minimal_status << ")";
            if (!agg.ns__value.empty())
            {
                os << indent << "{" << std::endl;
                for (const auto &[ns,value]: agg.ns__value)
                    os << indent << "  [Ns__Value]" << "(ns:" << ns << ")(value:" << value << ")" << std::endl;
                os <<  indent << "}" << std::endl;
            }
            if (!agg.ns__values.empty())
            {
                os << indent << "{" << std::endl;
                for (const auto &[ns,values]: agg.ns__values)
                {
                    os << indent << "  [Ns__Values]" << "(ns:" << ns << ")";
                    for (const auto &value: values)
                        os << "(value:" << value << ")";
                    os << std::endl;
                }
                os <<  indent << "}" << std::endl;
            }
            os << std::endl;
        };
        stream_agg("agg_local", agg_local);
        stream_agg("agg_global", agg_global);
    }

} }
