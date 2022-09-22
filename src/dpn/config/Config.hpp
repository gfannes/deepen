#ifndef HEADER_dpn_config_Config_hpp_ALREADY_INCLUDED
#define HEADER_dpn_config_Config_hpp_ALREADY_INCLUDED

#include <string>
#include <vector>
#include <map>

namespace dpn { namespace config { 

    class Config
    {
    public:
        std::vector<std::string> paths;
        std::map<std::string, std::string> name__path;
        std::vector<std::string> default_inputs;

        // Reads the config from NAFT format:
        // Ontology:
        // root:
        //   * path: path to root, mandatory
        //   * file: relative filepath to default input file, optional
        //   * name: name that can be used in links, optional
        // Eg,
        // [root](fp:/home/geertf/gubg)(name:gubg)
        // [root](fp:/home/geertf/decode-it/deepen)(name:dpn)
        bool load_from_file(const std::string &filepath);

        bool substitute_names(std::string &str) const;

    private:
    };

    bool get_default_config_filepath(std::string &filepath);

} }

#endif
