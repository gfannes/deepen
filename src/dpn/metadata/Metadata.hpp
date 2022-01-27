#ifndef HEADER_dpn_metadata_Metadata_hpp_ALREADY_INCLUDED
#define HEADER_dpn_metadata_Metadata_hpp_ALREADY_INCLUDED

#include <dpn/metadata/Item.hpp>
#include <dpn/metadata/Duration.hpp>
#include <dpn/metadata/Status.hpp>
#include <dpn/log.hpp>
#include <dpn/config/Config.hpp>

#include <gubg/mss.hpp>

#include <vector>
#include <set>
#include <map>
#include <optional>
#include <cmath>
#include <filesystem>

namespace dpn { namespace metadata { 

    using Ns__Value = std::map<std::string, std::string>;
    using Ns__Values = std::map<std::string, std::set<std::string>>;

    struct Input
    {
        std::optional<Duration> effort;
        std::optional<Status> status;
        std::optional<std::string> linkpath_rel;//Holds the path as specified in the []() link
        std::optional<std::filesystem::path> linkpath_abs;//Holds the absolute version, used for finding Nodes etc
        Ns__Value ns__value;
    };

    struct AggregatedUp
    {
        Status status;
        Ns__Value ns__value;
        Duration my_effort;
        Duration my_done;

        Duration my_todo() const {Duration d; d.minutes = my_effort.minutes-my_done.minutes; return d;}
    };

    struct AggregatedDown
    {
        Status minimal_status;
        Ns__Values ns__values;

        Duration total_effort;
        Duration total_done;

        Duration total_todo() const {Duration d; d.minutes = total_effort.minutes-total_done.minutes; return d;}
        double fraction_done() const {return total_effort.minutes > 0 ? total_done.minutes/total_effort.minutes : 1.0;}
        double pct_done() const {return std::lround(fraction_done()*1000.0)/10.0;}
    };

    class Metadata
    {
    public:
        std::optional<Item> link;
        std::set<Item> items;
        Input input;
        AggregatedUp   agg_up;//Metadata aggregated from input and parent, if present
        AggregatedDown agg_down_local;//Metadata aggregated from input, parent and childs, but locally within the given file
        AggregatedDown agg_down_global;//Metadata aggregated from input, parent, childs, over all linked files
        std::set<std::filesystem::path> linkpaths;

        void clear() {*this = Metadata{};}

        void setup(const std::vector<Item> &items, const config::Config &config, const std::filesystem::path &cwd);

        void aggregate_from_parent(const Metadata *parent, const Ns__Values &ns__possible_values);
        void aggregate_from_child(const Metadata &child);

        template <typename Ftor>
        bool aggregate_global(Ftor &&ftor)
        {
            MSS_BEGIN(bool);

            agg_down_global.minimal_status = agg_down_local.minimal_status;

            auto aggregate = [&](const AggregatedDown &local){
                agg_down_global.total_effort += local.total_effort;
                agg_down_global.total_done += local.total_done;
                agg_down_global.minimal_status = Status::minimum(agg_down_global.minimal_status, local.minimal_status);
                for (const auto &[ns,values]: local.ns__values)
                    agg_down_global.ns__values[ns].insert(values.begin(), values.end());
            };

            aggregate(agg_down_local);

            for (const auto &linkpath: linkpaths)
            {
                const AggregatedDown *agg_down_local_ptr = ftor(linkpath);
                MSS(!!agg_down_local_ptr, log::error() << "Could not find local aggregation for `" << linkpath << "`" << std::endl);

                aggregate(*agg_down_local_ptr);
            }

            MSS_END();
        }

        void stream(std::ostream &os, unsigned int level) const;

    private:
    };

    inline std::ostream &operator<<(std::ostream &os, const Metadata &metadata)
    {
        metadata.stream(os, 0);
        return os;
    }

} }

#endif
