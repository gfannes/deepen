#ifndef HEADER_dpn_Options_hpp_ALREADY_INCLUDED
#define HEADER_dpn_Options_hpp_ALREADY_INCLUDED

#include <dpn/enums.hpp>
#include <dpn/onto/enums.hpp>
#include <string>
#include <optional>
#include <list>

namespace dpn { 

    class Options
    {
    public:
        std::string exe_name;

        //Verb
        std::optional<Verb> verb_opt;

        //Options
        int verbosity_level = 0;

        std::list<std::string> input_filepaths;
        std::optional<std::string> output_filepath;

        std::list<std::string> tags;

        std::optional<onto::Format> format;

        //Arguments
        std::list<std::string> arguments;

        bool parse(int argc, const char **argv);
        std::string help() const;

        bool print_help() const {return verb_opt && *verb_opt == Verb::Help;}

    private:
    };

}

#endif
