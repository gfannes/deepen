#include <dpn/metadata/Metadata.hpp>

namespace dpn { namespace metadata { 

    void Metadata::setup(const std::vector<Item> &items, const config::Config &config, const std::filesystem::path &cwd)
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
                    std::filesystem::path filepath = config.substitute_names(item.value);
                    input.linkpath_rel = filepath.string();
                    if (!filepath.is_absolute())
                        filepath = cwd / filepath;
                    input.linkpath_abs = filepath.lexically_normal();
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

    void Metadata::aggregate_from_parent(const Metadata *parent, const Ns__Values &ns__possible_values)
    {
        //Non-effort aggregation
        {
            if (input.linkpath_abs)
                linkpaths.insert(*input.linkpath_abs);

            //Setup ns__value
            agg_up.ns__value = input.ns__value;
            //Aggregate ns__value UP
            if (parent)
            {
                for (const auto &[ns,value]: parent->agg_up.ns__value)
                {
                    if (!agg_up.ns__value.count(ns))
                        agg_up.ns__value[ns] = value;
                }
            }

            //Setup ns__values
            for (const auto &[ns,value]: input.ns__value)
                agg_down_local.ns__values[ns].insert(value);

            if (parent)
            {
                //Take status from input or parent, if present
                agg_up.status = (input.status ? *input.status : parent->agg_up.status);

                //Setup minimal_status
                agg_down_local.minimal_status = agg_up.status;
            }
        }

        //Effort aggregation
        {
            auto effort_minutes = (input.effort ? input.effort->minutes : 0);
            {
                //Check that for each of the namespaces in ns__possible_values, this MD has
                //a value that is in ns__possible_values[ns]. If not, this MD should be set to 0
                for (const auto &[ns,possible_values]: ns__possible_values)
                {
                    bool set_to_zero = true;
                    auto p = agg_up.ns__value.find(ns);
                    if (p != agg_up.ns__value.end())
                    {
                        set_to_zero = false;
                        const auto &actual_value = p->second;
                        if (possible_values.count(actual_value) == 0)
                            set_to_zero = true;
                    }
                    if (set_to_zero)
                        effort_minutes = 0;
                }
            }

            agg_up.my_effort.minutes = effort_minutes* agg_up.status.fraction_effort();
            agg_up.my_done.minutes = agg_up.my_effort.minutes*agg_up.status.fraction_done();

            agg_down_local.total_effort = agg_up.my_effort;
            agg_down_local.total_done   = agg_up.my_done;
        }
    }
    void Metadata::aggregate_from_child(const Metadata &child)
    {
        agg_down_local.minimal_status = Status::minimum(agg_down_local.minimal_status, child.agg_down_local.minimal_status);

        //Aggregate ns__values DOWN
        for (const auto &[ns,values]: child.agg_down_local.ns__values)
            agg_down_local.ns__values[ns].insert(values.begin(), values.end());

        agg_down_local.total_effort += child.agg_down_local.total_effort;
        agg_down_local.total_done += child.agg_down_local.total_done;
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

        auto stream_up = [&](const char *name, const auto &agg){
            os << indent << "[" << name << "]";
            os << "(my_effort:" << agg.my_effort << ")";
            os << "(my_done:" << agg.my_done << ")";
            os << "(status:" << agg.status << ")";
            if (!agg.ns__value.empty())
            {
                os << indent << "{" << std::endl;
                for (const auto &[ns,value]: agg.ns__value)
                    os << indent << "  [Ns__Value]" << "(ns:" << ns << ")(value:" << value << ")" << std::endl;
                os <<  indent << "}" << std::endl;
            }
            os << std::endl;
        };
        stream_up("agg_up", agg_up);

        auto stream_down = [&](const char *name, const auto &agg){
            os << indent << "[" << name << "]";
            os << "(total_effort:" << agg.total_effort << ")";
            os << "(total_done:" << agg.total_done << ")";
            os << "(minimal_status:" << agg.minimal_status << ")";
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
        stream_down("agg_down_local", agg_down_local);
        stream_down("agg_down_global", agg_down_global);
    }

} }
