#ifndef HEADER_dpn_metadata_Metadata_hpp_ALREADY_INCLUDED
#define HEADER_dpn_metadata_Metadata_hpp_ALREADY_INCLUDED

#include <dpn/metadata/Item.hpp>
#include <dpn/metadata/Duration.hpp>
#include <dpn/metadata/Status.hpp>
#include <dpn/log.hpp>
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

    struct Aggregated
    {
        Duration total_effort;
        Duration total_done;
        Status status;
        Status minimal_status;
        Ns__Value ns__value;
        Ns__Values ns__values;

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
        Aggregated agg_local;
        std::set<std::filesystem::path> linkpaths;
        Aggregated agg_global;

        void clear() {*this = Metadata{};}

        void setup(const std::vector<Item> &items, const std::filesystem::path &cwd);

        void setup_aggregated();
        void aggregate_from_parent(const Metadata &parent);
        void aggregate_from_child(const Metadata &child);
        void finalize_aggregated();

        template <typename Ftor>
        bool aggregate_global(Ftor &&ftor)
        {
            MSS_BEGIN(bool);

            agg_global.status = agg_local.status;
            agg_global.minimal_status = agg_local.minimal_status;

            auto aggregate = [&](const Aggregated &local){
                agg_global.total_effort += local.total_effort;
                agg_global.total_done += local.total_done;
                agg_global.minimal_status = Status::minimum(agg_global.minimal_status, local.minimal_status);
            };

            aggregate(agg_local);

            for (const auto &linkpath: linkpaths)
            {
                const Aggregated *agg_local_ptr = ftor(linkpath);
                MSS(!!agg_local_ptr, log::error() << "Could not find local aggregation for `" << linkpath << "`" << std::endl);

                aggregate(*agg_local_ptr);
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
