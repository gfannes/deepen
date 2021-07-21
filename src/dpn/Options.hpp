#ifndef HEADER_dpn_Options_hpp_ALREADY_INCLUDED
#define HEADER_dpn_Options_hpp_ALREADY_INCLUDED

#include <string>

namespace dpn { 

    class Options
    {
    public:
        std::string exe_name;

        bool print_help = false;
        int verbosity_level = 0;

        bool parse(int argc, const char **argv);
        std::string help() const;

    private:
    };

}

#endif
