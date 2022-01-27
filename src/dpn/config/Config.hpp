#ifndef HEADER_dpn_config_Config_hpp_ALREADY_INCLUDED
#define HEADER_dpn_config_Config_hpp_ALREADY_INCLUDED

#include <string>
#include <vector>
#include <map>

namespace dpn { namespace config { 

    class Config
    {
    public:
        std::vector<std::string> fps;
        std::map<std::string, std::string> name__fp;

        // Reads the config from NAFT format:
        // Ontology:
        // root:
        //   * fp: filepath, mandatory
        //   * name: name that can be used in links, optional
        // Eg,
        // [root](fp:/home/geertf/gubg)(name:gubg)
        // [root](fp:/home/geertf/decode-it/deepen)(name:dpn)
        bool load_from_file(const std::string &filepath);

        std::string substitute_names(const std::string &str) const;

    private:
    };

    bool get_default_config_filepath(std::string &filepath);

} }

#endif
