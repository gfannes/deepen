#ifndef HEADER_dpn_Options_hpp_ALREADY_INCLUDED
#define HEADER_dpn_Options_hpp_ALREADY_INCLUDED

#include <dpn/enums.hpp>
#include <dpn/onto/enums.hpp>
#include <dpn/meta/Moscow.hpp>
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
        std::optional<meta::Moscow> moscow;

        std::list<std::string> input_filepaths;
        std::optional<std::string> output_filepath;

        // .@ Remove tags_ and all its users
        std::list<std::string> tags_;//Old

        std::map<std::string, std::set<std::string>> incl_tags;//New
        std::map<std::string, std::set<std::string>> excl_tags;//New

        std::optional<onto::Format> format;

        std::set<unsigned int> details;
        bool all_details = false;
        bool detailed_ = false;

        std::optional<std::string> resources_fp;

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
