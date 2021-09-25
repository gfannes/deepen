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

        enum class State {Verb, Options, Arguments};
        State state = State::Verb;

        for (std::string arg, tmp; argr.pop(arg);)
        {
            auto is = [&](const char *sh, const char *lh){return arg == sh || arg == lh;};

            if (state == State::Verb)
            {
                if (false);
                else if (is("h", "help"))   {verb_opt = Verb::Help;}
                else if (is("l", "list"))   {verb_opt = Verb::List;}
                else if (is("u", "update")) {verb_opt = Verb::UpdateWithoutAggregates;}
                else if (is("U", "Update")) {verb_opt = Verb::UpdateWithAggregates;}
                else if (is("e", "export")) {verb_opt = Verb::Export;}
                else if (is("r", "run"))    {verb_opt = Verb::Run;}
                else {state = State::Options;}
            }

            if (state == State::Options)
            {
                if (false);
                else if (is("-h", "--help"))            {verb_opt = Verb::Help;}
                else if (is("-V", "--verbose"))         {MSS(argr.pop(verbosity_level),           log::error() << "Expected a valid verbosity level" << std::endl);}
                else if (is("-i", "--input"))           {MSS(argr.pop(tmp),                       log::error() << "Expected an input filepath" << std::endl); input_filepaths.push_back(tmp);}
                else if (is("-o", "--output"))          {MSS(argr.pop(output_filepath.emplace()), log::error() << "Expected an output filepath" << std::endl);}
                else if (is("-t", "--tag"))             {MSS(argr.pop(tmp),                       log::error() << "Expected a tag" << std::endl); tags.push_back(tmp);}
                else if (is("--", "--end"))             {state = State::Arguments;}
                else if (!arg.empty() && arg[0] == '-') {MSS(false,                               log::error() << "Unknown option " << arg << std::endl;);}
                else {state = State::Arguments;}
            }

            if (state == State::Arguments)
            {
                for (std::string arg; argr.pop(arg); )
                    arguments.push_back(arg);
                return true;
            }
        }

        MSS_END();
    }

    std::string Options::help() const
    {
        return std::string("Usage: ")+exe_name+R"eod( verb? option* argument*
Verbs:
    h     help                   Print this help
    l     list                   List unfinished items
    u     update                 Perform update operation, do not include aggregates
    U     Update                 Perform update operation, do include aggregates
    e     export                 Perform export operation
    r     run                    Run command composed from the arguments in each root folder
Options:
    -h    --help                 Print this help
    -V    --verbose <NUMBER>     Set verbosity level [default: 0]
    -i    --input   <FILEPATH>   Add input filepath
    -o    --output  <FILEPATH>   Set output filepath
    -t    --tag     <TAG>        Add tag
    --    --end                  All subsequent items will be interpreted as argument
Argument interpretation for verb:
    help                         Provide help on the specified verbs
    run                          Different items of the command to run
Written by Geert Fannes
)eod";
    }

}
