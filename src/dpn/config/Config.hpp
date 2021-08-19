#ifndef HEADER_dpn_config_Config_hpp_ALREADY_INCLUDED
#define HEADER_dpn_config_Config_hpp_ALREADY_INCLUDED

#include <string>
#include <vector>

namespace dpn { namespace config { 

    class Config
    {
    public:
        std::vector<std::string> roots;

        bool load_from_file(const std::string &filepath);

    private:
    };

    bool get_default_config_filepath(std::string &filepath);

} }

#endif
