#include <dpn/config/Config.hpp>
#include <dpn/log.hpp>
#include <gubg/naft/Range.hpp>
#include <gubg/file/system.hpp>
#include <gubg/mss.hpp>
#include <cstdlib>

namespace dpn { namespace config { 

    bool Config::load_from_file(const std::string &filepath)
    {
        MSS_BEGIN(bool);

        std::string content;
        MSS(gubg::file::read(content, filepath), log::error() << "Could not read from config file `" << filepath << "`" << std::endl);

        gubg::naft::Range range{content};
        for (std::string tag; !range.empty() && range.pop_tag(tag); )
        {
            if (false) {}
            else if (tag == "root")
            {
                for (std::string k,v; range.pop_attr(k,v); )
                {
                    if (false) {}
                    else if (k == "fp") roots.push_back(v);
                    else MSS(false, log::error() << "Unknown attribute `" << k << ":" << v << "` for tag `" << tag << "` in `" << filepath << "`" << std::endl);
                }
            }
            else MSS(false, log::error() << "Unknown config tag `" << tag << "` in `" << filepath << "`" << std::endl);
        }

        MSS_END();
    }

    bool get_default_config_filepath(std::string &filepath)
    {
        MSS_BEGIN(bool);

        std::string home;
        {
            if (const auto home_cstr = std::getenv("HOME"); !!home_cstr)
            {
                home = home_cstr;
            }
            else if (const auto homedrive = std::getenv("HOMEDRIVE"), homepath = std::getenv("HOMEPATH"); !!homedrive && !!homepath)
            {
                home = homedrive;
                home += homepath;
            }
            else
                MSS(false, log::error() << "Could not find the HOME folder" << std::endl);
        }

        filepath = home + "/.config/dpn/config.naft";

        MSS_END();
    }

} }