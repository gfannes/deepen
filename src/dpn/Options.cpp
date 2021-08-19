#include <dpn/Options.hpp>
#include <dpn/log.hpp>
#include <gubg/mss.hpp>

namespace dpn { 

    bool Options::parse(int argc, const char **argv)
    {
        MSS_BEGIN(bool);

        unsigned int arg_ix = 0;
        auto pop_str = [&](std::string &str)
        {
            if (arg_ix >= argc) return false;
            str = argv[arg_ix++];
            return true;
        };

        auto pop_num = [&](auto &num)
        {
            std::string str;
            if (!pop_str(str)) return false;

            try { num = std::stol(str); }
            catch (...) { return (log::error() << "Could not convert `" << str << "` to a number" << std::endl, false); }

            return true;
        };

        MSS(pop_str(exe_name), log::error() << "Could not find the executable name" << std::endl);

        for (std::string arg; pop_str(arg);)
        {
            auto is = [&](const char *sh, const char *lh){return arg == sh || arg == lh;};

            if (false) {}
            else if (is("-h", "--help")) { print_help = true; }
            else if (is("-V", "--verbose")) { MSS(pop_num(verbosity_level), log::error() << "Expected a verbosity level" << std::endl); }
            else if (is("-u", "--update")) { operation_opt = Operation::Update; }
            else if (is("-e", "--export")) { operation_opt = Operation::Export; }
            else if (is("-i", "--input")) { MSS(pop_str(input_filepath), log::error() << "Expected an input filepath" << std::endl); }
            else if (is("-o", "--output")) { MSS(pop_str(output_filepath), log::error() << "Expected an output filepath" << std::endl); }
            else if (is("-c", "--command")) { command.emplace(); MSS(pop_str(*command), log::error() << "Expected a command string" << std::endl); }
            else { log::error() << "Unknown CLI argument `" << arg << "`" << std::endl; }
        }

        MSS_END();
    }

    std::string Options::help() const
    {
        return std::string("Help for ")+exe_name+R"eod(
-h    --help                Print this help
-V    --verbose <NUMBER>    Set verbosity level [default: 0]
-u    --update              Perform update operation
-e    --export              Perform export operation
-i    --input   <FILEPATH>  Set input filepath
-o    --output  <FILEPATH>  Set output filepath
-c    --command <STRING>    Run command in each root folder
Written by Geert Fannes
)eod";
    }

}
