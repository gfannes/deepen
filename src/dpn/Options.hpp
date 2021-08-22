#ifndef HEADER_dpn_Options_hpp_ALREADY_INCLUDED
#define HEADER_dpn_Options_hpp_ALREADY_INCLUDED

#include <dpn/enums.hpp>
#include <string>
#include <optional>
#include <list>

namespace dpn { 

    class Options
    {
    public:
        std::string exe_name;

        bool print_help = false;
        int verbosity_level = 0;

        std::optional<Operation> operation_opt;
        bool include_aggregates = false;

        std::list<std::string> input_filepaths;
        std::string output_filepath;

        std::optional<std::string> command;

        bool parse(int argc, const char **argv);
        std::string help() const;

    private:
    };

}

#endif
