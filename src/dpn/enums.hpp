#ifndef HEADER_dpn_enums_hpp_ALREADY_INCLUDED
#define HEADER_dpn_enums_hpp_ALREADY_INCLUDED

namespace dpn { 

    enum class Verb
    {
        Help, UpdateWithoutAggregates, UpdateWithAggregates, Export, List, Run, Show,
    };

    enum class Show
    {
        Inbox, Actionable, Forwarded, WIP, DueDate, Features, Todo, KeyValues, KeyValues_v, Debug,
    };

    enum class Direction
    {
        Push, Pull
    };

    enum class Sort
    {
        Effort, Rice, DueDate,
    };

}

#endif
