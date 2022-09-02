#include <dpn/Options.hpp>
#include <dpn/log.hpp>
#include <gubg/cli/Range.hpp>
#include <gubg/mss.hpp>
#include <termcolor/termcolor.hpp>
#include <sstream>
#include <iomanip>

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
            for (bool arg_was_recognised = false; !arg_was_recognised; )
            {
                auto is = [&](const char *sh, const char *lh)
                {
                    arg_was_recognised = (arg == sh || arg == lh);
                    return arg_was_recognised;
                };

                switch (state)
                {
                    case State::Verb:
                    if (false);
                    else if (is("h", "help"))   {verb_opt = Verb::Help;}
                    else if (is("l", "list"))   {verb_opt = Verb::List;}
                    else if (is("u", "update")) {verb_opt = Verb::UpdateWithoutAggregates;}
                    else if (is("U", "Update")) {verb_opt = Verb::UpdateWithAggregates;}
                    else if (is("e", "export")) {verb_opt = Verb::Export;}
                    else if (is("r", "run"))    {verb_opt = Verb::Run;}
                    else if (is("D", "debug"))  {verb_opt = Verb::PrintDebug;}
                    else if (is("i", "inbox"))  {verb_opt = Verb::Inbox;}
                    else if (is("a", "action")) {verb_opt = Verb::Actionable;}
                    else if (is("f", "forward")){verb_opt = Verb::Forwarded;}
                    else if (is("w", "wip"))    {verb_opt = Verb::WIP;}
                    else if (is("d", "due"))    {verb_opt = Verb::Duedate;}
                    else if (is("p", "project")){verb_opt = Verb::Projects;}
                    else if (is("t", "todo"))   {verb_opt = Verb::Todo;}
                    state = State::Options;
                    break;

                    case State::Options:
                    if (false);
                    else if (is("-h", "--help"))            {verb_opt = Verb::Help;}
                    else if (is("-d", "--detailed"))        {detailed = true;}
                    else if (is("-V", "--verbose"))         {MSS(argr.pop(verbosity_level),           log::error() << "Expected a valid verbosity level" << std::endl);}
                    else if (is("-i", "--input"))           {MSS(argr.pop(tmp),                       log::error() << "Expected an input filepath" << std::endl); input_filepaths.push_back(tmp);}
                    else if (is("-o", "--output"))          {MSS(argr.pop(output_filepath.emplace()), log::error() << "Expected an output filepath" << std::endl);}
                    else if (is("-t", "--tag"))
                    {
                        MSS(argr.pop(tmp), log::error() << "Expected a tag" << std::endl);
                        tags_.push_back(tmp);
                        const auto ix = tmp.find(':');
                        if (ix == std::string::npos)
                            tags[tmp] = "";
                        else
                            tags[tmp.substr(0, ix)] = tmp.substr(ix+1);
                    }
                    else if (is("-f", "--format"))
                    {
                        MSS(argr.pop(tmp),                                                            log::error() << "Expected a tag" << std::endl);
                        MSS(onto::parse_format(tmp, [&](auto fmt){format = fmt;}),                    log::error() << "Unknown format '" << tmp << "'" << std::endl);
                    }
                    else if (is("--", "--end"))             {state = State::Arguments;}
                    else if (!arg.empty() && arg[0] == '-') {MSS(false,                               log::error() << "Unknown option " << arg << std::endl;);}
                    else {state = State::Arguments;}
                    break;

                    case State::Arguments:
                    arguments.push_back(arg);
                    for (std::string arg; argr.pop(arg); )
                        arguments.push_back(arg);
                    arg_was_recognised = true;
                    break;
                }
            }
        }

        MSS_END();
    }

    bool Options::rearrange()
    {
        MSS_BEGIN(bool);

        if (verb_opt)
        {
            switch (*verb_opt)
            {
                case Verb::UpdateWithAggregates:
                case Verb::UpdateWithoutAggregates:
                input_filepaths.splice(input_filepaths.end(), arguments);
                break;

                case Verb::List:
                break;

                default: break;
            }
        }

        MSS_END();
    }

    std::string Options::help() const
    {
        std::ostringstream oss; oss << termcolor::colorize;

        oss << "Usage: " << termcolor::green << exe_name << " verb? option* argument*" << termcolor::reset << std::endl;

        auto option = [&](auto sh, auto lh, auto opt, auto expl){
            oss                      << std::setw(4)               << " ";
            oss << termcolor::yellow << std::setw(6)  << std::left << sh << termcolor::reset;
            oss << termcolor::yellow << std::setw(12) << std::left << lh << termcolor::reset;
            oss << termcolor::blue   << std::setw(12) << std::left << opt << termcolor::reset;
            oss                      << std::setw(6)  << std::left << expl;
            oss << std::endl;
        };

        oss << "Verbs:" << std::endl;
        option("h", "help", "", "Print this help");
        option("l", "list", "", "List unfinished items");
        option("u", "update", "", "Perform update operation, do not include aggregates");
        option("U", "Update", "", "Perform update operation, do include aggregates");
        option("e", "export", "", "Perform export operation");
        option("r", "run", "", "Run command composed from the arguments in each root folder");
        option("D", "debug", "", "Print debug version of loaded library");
        option("i", "inbox", "", "Work on Inbox items");
        option("a", "action", "", "Work on Actionable items");
        option("f", "forward", "", "Work on Forwarded items");
        option("w", "wip", "", "Work on WIP items");
        option("d", "due", "", "Work on items with Duedate");
        option("p", "project", "", "List Projects");
        option("t", "todo", "", "List Todo");

        oss << "Options:" << std::endl;
        option("-h", "--help", "", "Print this help");
        option("-d", "--detailed", "", "Provide detailed information");
        option("-V", "--verbose", "<NUMBER>", "Set verbosity level [default: 0]");
        option("-i", "--input", "<FILEPATH>", "Add input filepath");
        option("-o", "--output", "<FILEPATH>", "Set output filepath");
        option("-t", "--tag", "<TAG>", "Add tag");
        option("-f", "--format", "<STRING>", "Output format (md|jira|textile)");
        option("--", "--end", "", "All subsequent items will be interpreted as argument");

        oss << "Argument interpretation for verb:" << std::endl;
        option("", "help", "", "Provide help on the specified verbs");
        option("", "run", "", "Different items of the command to run");
        option("", "update|Update", "", "Input filepaths");

        oss << "Status indication, use `@X~` to indicate as `cancelled`:" << std::endl;
        auto status = [&](bool header, const char *wip, const char *done, const char *expl){
            oss                      << std::setw(4)               << " ";
            oss << (header ? termcolor::blue<char> : termcolor::yellow<char>) << std::setw(6)  << std::left << wip << termcolor::reset;
            oss << (header ? termcolor::blue<char> : termcolor::yellow<char>) << std::setw(12) << std::left << done << termcolor::reset;
            oss                      << std::setw(6)  << std::left << expl;
            oss << std::endl;
        };
        status(true, "WIP", "DONE", "Description");
        status(false, "@r", "@R", "[R]equirements collection");
        status(false, "@d", "@D", "[D]esigning the solution");
        status(false, "@s", "@S", "[S]tarting development");
        status(false, "@i", "@I", "[I]mplementing the solution");
        status(false, "@v", "@V", "[V]alidation");

        oss << "Written by Geert Fannes" << std::endl;
        return oss.str();
    }

}
