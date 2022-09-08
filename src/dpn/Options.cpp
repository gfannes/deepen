#include <dpn/Options.hpp>
#include <dpn/log.hpp>

#include <gubg/cli/Range.hpp>
#include <gubg/Strange.hpp>
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
                    else if (is("s", "show"))
                    {
                        verb_opt = Verb::Show;
                        MSS(argr.pop(tmp), log::error() << "You have to indicate what you want to show" << std::endl);
                        auto set_show_if = [&](const std::string &str, auto show){
                            if (str == tmp)
                                show_opt = show;
                            return !!show_opt;
                        };
                        MSS(set_show_if("inbox", Show::Inbox)
                            || set_show_if("action", Show::Actionable)
                            || set_show_if("forward", Show::Forwarded)
                            || set_show_if("wip", Show::WIP)
                            || set_show_if("done", Show::Done)
                            || set_show_if("due", Show::DueDate)
                            || set_show_if("feature", Show::Features)
                            || set_show_if("todo", Show::Todo)
                            || set_show_if("kv", Show::KeyValues)
                            || set_show_if("KV", Show::KeyValues_v)
                            || set_show_if("debug", Show::Debug)
                            , log::error() << "Don't know how to show '" << tmp << "'" << std::endl
                            );
                    }
                    state = State::Options;
                    break;

                    case State::Options:
                    if (false);
                    else if (is("-h", "--help"))            {verb_opt = Verb::Help;}
                    else if (is("-D", "--detailed"))        {detailed_ = true;}
                    else if (is("-V", "--verbose"))         {MSS(argr.pop(verbosity_level),           log::error() << "Expected a valid verbosity level" << std::endl);}
                    else if (is("-i", "--input"))           {MSS(argr.pop(tmp),                       log::error() << "Expected an input filepath" << std::endl); input_filepaths.push_back(tmp);}
                    else if (is("-o", "--output"))          {MSS(argr.pop(output_filepath.emplace()), log::error() << "Expected an output filepath" << std::endl);}
                    else if (is("-k", "--color_output"))    {MSS(argr.pop(color_output),              log::error() << "Expected a boolean" << std::endl);}
                    else if (is("-r", "--reverse"))         {reverse = true;}
                    else if (is("-s", "--sort"))
                    {
                        MSS(argr.pop(tmp), log::error() << "Expected a string" << std::endl);
                        auto set_sort_if = [&](const char *str, Sort s){
                            if (tmp == str)
                                sort = s;
                            return !!sort;
                        };
                        MSS(set_sort_if("no", Sort::Effort)
                            || set_sort_if("effort", Sort::Effort)
                            || set_sort_if("urgency", Sort::Urgency)
                            || set_sort_if("rice", Sort::Rice)
                            || set_sort_if("due", Sort::DueDate)
                            , log::error() << "Unknown sort type '" << tmp << "'" << std::endl);
                    }
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
                    else if (is("-d", "--details"))
                    {
                        MSS(argr.pop(tmp), log::error() << "Expected a list of uint" << std::endl);
                        gubg::Strange strange{tmp};
                        for (unsigned int ui; strange.pop_decimal(ui); )
                        {
                            details.insert(ui);
                            MSS(strange.pop_if(',') || strange.empty(), log::error() << "The details list should be comma-separated" << std::endl);
                        }
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
            oss << termcolor::yellow << std::setw(16) << std::left << lh << termcolor::reset;
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
        option("s", "show", "inbox|action|forward|wip|done|due|feature|todo|kv|KV", "Show list or items");

        oss << "Options:" << std::endl;
        option("-h", "--help", "", "Print this help");
        option("-D", "--detailed", "", "Provide detailed information");
        option("-V", "--verbose", "<NUMBER>", "Set verbosity level [default: 0]");
        option("-i", "--input", "<FILEPATH>", "Add input filepath");
        option("-o", "--output", "<FILEPATH>", "Set output filepath");
        option("-k", "--color_output", "<BOOLEAN>", "Set colored output [default: yes]");
        option("-r", "--reverse", "", "Show items in reversed order [default: no]");
        option("-t", "--tag", "<STRING>:<STRING>", "Add key-value tag");
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
