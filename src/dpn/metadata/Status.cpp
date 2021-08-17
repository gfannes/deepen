#include <dpn/metadata/Status.hpp>

namespace dpn { namespace metadata { 

    double Status::fraction_done() const
    {
        switch (state)
        {
            case State::Requirement:    return (done ? 0.2 : 0.0); break;
            case State::Design:         return (done ? 0.4 : 0.2); break;
            case State::Starting:       return (done ? 0.6 : 0.4); break;
            case State::Implementation: return (done ? 0.8 : 0.6); break;
            case State::Validation:     return (done ? 1.0 : 0.8); break;
        }
        return 0.0;
    }

    bool Status::parse(gubg::Strange &strange)
    {
        if (strange.empty())
            return false;

        switch (strange.front())
        {
            case 'r': *this = Status{State::Requirement,    false}; break;
            case 'R': *this = Status{State::Requirement,    true};  break;
            case 'd': *this = Status{State::Design,         false}; break;
            case 'D': *this = Status{State::Design,         true};  break;
            case 's': *this = Status{State::Starting,       false}; break;
            case 'S': *this = Status{State::Starting,       true};  break;
            case 'i': *this = Status{State::Implementation, false}; break;
            case 'I': *this = Status{State::Implementation, true};  break;
            case 'v': *this = Status{State::Validation,     false}; break;
            case 'V': *this = Status{State::Validation,     true};  break;

            default: return false; break;
        }

        strange.pop_front();
        return true;
    }

    void Status::stream(std::ostream &os) const
    {
        switch (state)
        {
            case State::Requirement:    os << (done ? 'R' : 'r'); break;
            case State::Design:         os << (done ? 'D' : 'd'); break;
            case State::Starting:       os << (done ? 'S' : 's'); break;
            case State::Implementation: os << (done ? 'I' : 'i'); break;
            case State::Validation:     os << (done ? 'V' : 'v'); break;
        }
    }

    Status Status::minimum(const Status &lhs, const Status &rhs)
    {
        const auto lhs_state = (int)lhs.state;
        const auto rhs_state = (int)rhs.state;

        if (lhs_state < rhs_state)
            return lhs;
        if (lhs_state == rhs_state)
            return Status{(State)lhs_state, std::min(lhs.done, rhs.done)};
        return rhs;
    }

    bool Status::operator==(const Status &rhs) const
    {
        if (state != rhs.state)
            return false;
        if (done != rhs.done)
            return false;
        return true;
    }

} }
