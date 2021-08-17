#ifndef HEADER_dpn_metadata_Duration_hpp_ALREADY_INCLUDED
#define HEADER_dpn_metadata_Duration_hpp_ALREADY_INCLUDED

#include <gubg/Strange.hpp>
#include <ostream>

namespace dpn { namespace metadata { 

    class Duration
    {
    public:
        double minutes = 0.0;

        bool parse(gubg::Strange &strange);
        void stream(std::ostream &os) const;

        Duration &operator+=(const Duration &rhs);

    private:
    };

    inline std::ostream &operator<<(std::ostream &os, const Duration &duration)
    {
        duration.stream(os);
        return os;
    }

} }

#endif
