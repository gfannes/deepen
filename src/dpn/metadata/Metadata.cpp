#include <dpn/metadata/Metadata.hpp>

namespace dpn { namespace metadata { 

    void Metadata::setup(const std::vector<Item> &items)
    {
        clear();

        auto goc_input = [&]() -> Input& {
            if (!input_opt)
                input_opt.emplace();
            return *input_opt;
        };

        Duration effort;
        Status status;
        for (const auto &item: items)
        {
            if (item.key.empty())
            {
                gubg::Strange strange{item.value};
                if (false) {}
                else if (effort.parse(strange))
                    goc_input().effort = effort;
                else if (status.parse(strange))
                    goc_input().status = status;
                else
                    this->items.insert(item);
            }
            else
                this->items.insert(item);
        }
    }

    void Metadata::setup_aggregated()
    {
        if (input_opt)
        {
            const auto &input = *input_opt;
            aggregated_opt.emplace();
            auto &aggregated = *aggregated_opt;
            aggregated.total_effort = input.effort;
            aggregated.total_done.minutes = input.effort.minutes*input.status.fraction_done();
            aggregated.minimal_status = input.status;
        }
    }
    void Metadata::aggregate_from_parent(const Metadata &parent)
    {
    }
    void Metadata::aggregate_from_child(const Metadata &child)
    {
        if (child.aggregated_opt)
        {
            const auto &child_agg = *child.aggregated_opt;
            if (!aggregated_opt)
            {
                aggregated_opt = child_agg;
            }
            else
            {
                auto &my_agg = *aggregated_opt;
                my_agg.total_effort += child_agg.total_effort;
                my_agg.total_done += child_agg.total_done;
                my_agg.minimal_status = Status::minimum(my_agg.minimal_status, child_agg.minimal_status);
            }
        }
    }
    void Metadata::finalize_aggregated()
    {
        if (aggregated_opt)
        {
            auto &aggregated = *aggregated_opt;
            aggregated.total_todo.minutes = aggregated.total_effort.minutes-aggregated.total_done.minutes;
            if (aggregated.total_effort.minutes > 0)
                aggregated.fraction_done = aggregated.total_done.minutes/aggregated.total_effort.minutes;
            else
                aggregated.fraction_done = 1.0;
        }
    }

    void Metadata::stream(std::ostream &os, unsigned int level) const
    {
        const std::string indent(level*2, ' ');

        if (!items.empty())
        {
            for (const auto &item: items)
                os << indent << "[metadata::Item](key:" << item.key << ")(value:" << item.value << ")" << std::endl;
        }

        if (input_opt)
        {
            const auto &input = *input_opt;
            os << indent << "[metadata::Input](effort:" << input.effort << ")(status:" << input.status << ")" << std::endl;
        }

        if (aggregated_opt)
        {
            const auto &agg = *aggregated_opt;
            os << indent << "[metadata::Aggregated](total_effort:" << agg.total_effort << ")(total_done:" << agg.total_done << ")(total_todo:" << agg.total_todo << ")(pct_done:" << agg.pct_done() << "%)(minimal_status:" << agg.minimal_status << ")" << std::endl;
        }
    }

} }
