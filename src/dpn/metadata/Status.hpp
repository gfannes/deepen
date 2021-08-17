#ifndef HEADER_dpn_metadata_Status_hpp_ALREADY_INCLUDED
#define HEADER_dpn_metadata_Status_hpp_ALREADY_INCLUDED

#include <dpn/metadata/enums.hpp>
#include <gubg/Strange.hpp>
#include <ostream>

namespace dpn { namespace metadata { 

    class Status
    {
    public:
        State state = State::Requirement;
        bool done = false;

        Status() {}
        Status(State state, bool done): state(state), done(done) {}

        double fraction_done() const;

        bool parse(gubg::Strange &strange);
        void stream(std::ostream &os) const;

        static Status minimum(const Status &lhs, const Status &rhs);

        bool operator==(const Status &rhs) const;
        bool operator!=(const Status &rhs) const {return !operator==(rhs);}
    };

    inline std::ostream &operator<<(std::ostream &os, const Status &status)
    {
        status.stream(os);
        return os;
    }

} }

#endif
