#ifndef HEADER_dpn_Options_hpp_ALREADY_INCLUDED
#define HEADER_dpn_Options_hpp_ALREADY_INCLUDED

#include <dpn/enums.hpp>
#include <string>
#include <optional>

namespace dpn { 

    class Options
    {
    public:
        std::string exe_name;

        bool print_help = false;
        int verbosity_level = 0;

        std::optional<Operation> operation_opt;

        std::string input_filepath;
        std::string output_filepath;

        bool parse(int argc, const char **argv);
        std::string help() const;

    private:
    };

}

#endif
