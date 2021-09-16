#include <dpn/Options.hpp>
#include <dpn/log.hpp>
#include <gubg/cli/Range.hpp>
#include <gubg/mss.hpp>

namespace dpn { 

    bool Options::parse(int argc, const char **argv)
    {
        MSS_BEGIN(bool);

        gubg::cli::Range argr{argc, argv};

        MSS(argr.pop(exe_name), log::error() << "Could not find the executable name" << std::endl);

        for (std::string arg; argr.pop(arg);)
        {
            auto is = [&](const char *sh, const char *lh){return arg == sh || arg == lh;};

            if (false) {}
            else if (is("-h", "--help")) { print_help = true; }
            else if (is("-V", "--verbose")) { MSS(argr.pop(verbosity_level), log::error() << "Expected a valid verbosity level" << std::endl); }
            else if (is("-u", "--update")) { operation_opt = Operation::Update; include_aggregates = false; }
            else if (is("-U", "--Update")) { operation_opt = Operation::Update; include_aggregates = true; }
            else if (is("-e", "--export")) { operation_opt = Operation::Export; }
            else if (is("-i", "--input")) { std::string tmp; MSS(argr.pop(tmp), log::error() << "Expected an input filepath" << std::endl); input_filepaths.push_back(tmp); }
            else if (is("-o", "--output")) { MSS(argr.pop(output_filepath), log::error() << "Expected an output filepath" << std::endl); }
            else if (is("-c", "--command"))
            {
                command_args_opt.emplace();
                for (std::string arg; argr.pop(arg); command_args_opt->push_back(arg)){}
            }
            else { log::error() << "Unknown CLI argument `" << arg << "`" << std::endl; }
        }

        MSS_END();
    }

    std::string Options::help() const
    {
        return std::string("Help for ")+exe_name+R"eod(
-h    --help                 Print this help
-V    --verbose <NUMBER>     Set verbosity level [default: 0]
-u    --update               Perform update operation, do not include aggregates
-U    --Update               Perform update operation, do include aggregates
-e    --export               Perform export operation
-i    --input   <FILEPATH>   Set input filepath
-o    --output  <FILEPATH>   Set output filepath
-c    --command <ARGUMENT>*  Run command in each root folder, this will consume all subsequent arguments
Written by Geert Fannes
)eod";
    }

}
