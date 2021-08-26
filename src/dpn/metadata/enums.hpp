#ifndef HEADER_dpn_metadata_enums_hpp_ALREADY_INCLUDED
#define HEADER_dpn_metadata_enums_hpp_ALREADY_INCLUDED

namespace dpn { namespace metadata { 

    enum class Flow
    {
        Requirement, Design, Starting, Implementation, Validation,
    };

    enum class State
    {
        Active, Cancelled,
    };

} }

#endif
