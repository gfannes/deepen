#ifndef HEADER_dpn_log_hpp_ALREADY_INCLUDED
#define HEADER_dpn_log_hpp_ALREADY_INCLUDED

#include <ostream>

namespace dpn { namespace log { 

    std::ostream &error();
    std::ostream &warning();

    void set_verbosity_level(int verbosity_level);

    std::ostream &os(int message_importance);

} }

#endif
