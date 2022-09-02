#ifndef HEADER_dpn_enums_hpp_ALREADY_INCLUDED
#define HEADER_dpn_enums_hpp_ALREADY_INCLUDED

namespace dpn { 

    enum class Verb
    {
        Help, UpdateWithoutAggregates, UpdateWithAggregates, Export, List, Run, PrintDebug,
        Inbox, Actionable, Forwarded, WIP, Duedate, Projects, Todo,
    };

}

#endif
