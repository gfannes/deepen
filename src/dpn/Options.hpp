#ifndef HEADER_dpn_Options_hpp_ALREADY_INCLUDED
#define HEADER_dpn_Options_hpp_ALREADY_INCLUDED

#include <dpn/enums.hpp>
#include <string>
#include <optional>
#include <vector>
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
        std::optional<std::string> output_filepath;

        std::vector<std::string> tags;

        std::optional<std::vector<std::string>> command_args_opt;

        bool parse(int argc, const char **argv);
        std::string help() const;

    private:
    };

}

#endif
