#include <dpn/metadata/Status.hpp>

namespace dpn { namespace metadata { 

    double Status::fraction_done_raw_() const
    {
        switch (flow)
        {
            case Flow::Requirement:    return (done ? 0.2 : 0.0); break;
            case Flow::Design:         return (done ? 0.4 : 0.2); break;
            case Flow::Starting:       return (done ? 0.6 : 0.4); break;
            case Flow::Implementation: return (done ? 0.8 : 0.6); break;
            case Flow::Validation:     return (done ? 1.0 : 0.8); break;
        }
        return 0.0;
    }

    double Status::fraction_done() const
    {
        switch (state)
        {
            case State::Active: return fraction_done_raw_(); break;
            case State::Cancelled: return 1.0; break;
        }
        return 1.0;
    }

    double Status::fraction_effort() const
    {
        switch (state)
        {
            case State::Active: return 1.0; break;
            case State::Cancelled: return fraction_done_raw_(); break;
        }
        return 1.0;
    }

    bool Status::parse(gubg::Strange &strange)
    {
        if (strange.empty())
            return false;

        switch (strange.front())
        {
            case 'r': *this = Status{Flow::Requirement,    false}; break;
            case 'R': *this = Status{Flow::Requirement,    true};  break;
            case 'd': *this = Status{Flow::Design,         false}; break;
            case 'D': *this = Status{Flow::Design,         true};  break;
            case 's': *this = Status{Flow::Starting,       false}; break;
            case 'S': *this = Status{Flow::Starting,       true};  break;
            case 'i': *this = Status{Flow::Implementation, false}; break;
            case 'I': *this = Status{Flow::Implementation, true};  break;
            case 'v': *this = Status{Flow::Validation,     false}; break;
            case 'V': *this = Status{Flow::Validation,     true};  break;

            default: return false; break;
        }
        strange.pop_front();

        if (false) {}
        else if (strange.pop_if('~'))
            state = State::Cancelled;
        else if (strange.pop_if('x'))
            state = State::Blocked;
        else
            state = State::Active;

        return true;
    }

    void Status::stream(std::ostream &os) const
    {
        os << to_string();
    }
    std::string Status::to_string() const
    {
        std::string str;
        switch (flow)
        {
            case Flow::Requirement:    str.push_back(done ? 'R' : 'r'); break;
            case Flow::Design:         str.push_back(done ? 'D' : 'd'); break;
            case Flow::Starting:       str.push_back(done ? 'S' : 's'); break;
            case Flow::Implementation: str.push_back(done ? 'I' : 'i'); break;
            case Flow::Validation:     str.push_back(done ? 'V' : 'v'); break;
        }
        switch (state)
        {
            case State::Active: break;
            case State::Cancelled: str.push_back('~'); break;
            case State::Blocked: str.push_back('x'); break;
        }
        return str;
    }

    Status Status::minimum(const Status &lhs, const Status &rhs)
    {
        const auto lhs_flow = (int)lhs.flow;
        const auto rhs_flow = (int)rhs.flow;

        if (lhs_flow < rhs_flow)
            return lhs;
        if (lhs_flow == rhs_flow)
            return Status{(Flow)lhs_flow, std::min(lhs.done, rhs.done)};
        return rhs;
    }

    bool Status::operator==(const Status &rhs) const
    {
        if (flow != rhs.flow)
            return false;
        if (state != rhs.state)
            return false;
        return true;
    }

} }
