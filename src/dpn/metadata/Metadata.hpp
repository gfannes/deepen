#ifndef HEADER_dpn_metadata_Metadata_hpp_ALREADY_INCLUDED
#define HEADER_dpn_metadata_Metadata_hpp_ALREADY_INCLUDED

#include <dpn/metadata/Item.hpp>
#include <dpn/metadata/Duration.hpp>
#include <dpn/metadata/Status.hpp>
#include <vector>
#include <set>
#include <optional>
#include <cmath>

namespace dpn { namespace metadata { 

    struct Input
    {
        Duration effort;
        Status status;
    };

    struct Aggregated
    {
        Duration total_effort;
        Duration total_done;
        Duration total_todo;
        double fraction_done = 0.0;
        Status minimal_status;

        double pct_done() const {return std::lround(fraction_done*1000.0)/10.0;}
    };

    class Metadata
    {
    public:
        std::set<Item> items;
        std::optional<Input> input_opt;
        std::optional<Aggregated> aggregated_opt;

        void clear() {*this = Metadata{};}

        void setup(const std::vector<Item> &items);

        void setup_aggregated();
        void aggregate_from_parent(const Metadata &parent);
        void aggregate_from_child(const Metadata &child);
        void finalize_aggregated();

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
