#ifndef HEADER_dpn_enums_hpp_ALREADY_INCLUDED
#define HEADER_dpn_enums_hpp_ALREADY_INCLUDED

#include <string>
#include <ostream>

namespace dpn { 

    enum class Verb
    {
        Help, UpdateWithoutAggregates, UpdateWithAggregates, Export, List, Run, Show, Plan,
    };

    enum class Show
    {
        Inbox, Actionable, Forwarded, WIP, Done, DueDate, Features, Todo, KeyValues, KeyValues_v, Debug,
    };

    enum class Direction
    {
        Push, Pull,
    };

    enum class Dependency
    {
        None, Include, Require, Mine,
    };

    enum class Sort
    {
        No, Effort, Urgency, Rice, DueDate,
    };
    inline std::string to_string(Sort e)
    {
        switch (e)
        {
            case Sort::No: return "No"; break;
            case Sort::Effort: return "Effort"; break;
            case Sort::Urgency: return "Urgency"; break;
            case Sort::Rice: return "Rice"; break;
            case Sort::DueDate: return "DueDate"; break;
        }
        return "<Unknown Sort>";
    }
    inline std::ostream &operator<<(std::ostream &os, Sort e) {return os << to_string(e);}

}

#endif
