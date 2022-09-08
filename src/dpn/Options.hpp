#ifndef HEADER_dpn_Options_hpp_ALREADY_INCLUDED
#define HEADER_dpn_Options_hpp_ALREADY_INCLUDED

#include <dpn/enums.hpp>
#include <dpn/onto/enums.hpp>
#include <string>
#include <optional>
#include <list>
#include <set>
#include <map>

namespace dpn { 

    class Options
    {
    public:
        std::string exe_name;

        //Verb
        std::optional<Verb> verb_opt;

        //Options
        int verbosity_level = 0;
        bool color_output = true;
        std::optional<Sort> sort;
        bool reverse = false;

        std::list<std::string> input_filepaths;
        std::optional<std::string> output_filepath;

        // .@ Remove tags_ and all its users
        std::list<std::string> tags_;//Old

        std::map<std::string, std::string> tags;//New

        std::optional<onto::Format> format;

        std::set<unsigned int> details;
        bool detailed_ = false;

        //Arguments
        std::list<std::string> arguments;

        std::optional<Show> show_opt;

        bool parse(int argc, const char **argv);
        bool rearrange();
        std::string help() const;

        bool print_help() const {return verb_opt && *verb_opt == Verb::Help;}

    private:
    };

}

#endif
