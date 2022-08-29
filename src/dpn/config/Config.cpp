#include <dpn/config/Config.hpp>
#include <dpn/log.hpp>

#include <gubg/naft/Range.hpp>
#include <gubg/file/system.hpp>
#include <gubg/mss.hpp>

#include <optional>
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
                std::optional<std::string> path_opt;
                std::optional<std::string> file_opt;
                std::optional<std::string> name_opt;
                for (std::string k,v; range.pop_attr(k,v); )
                {
                    if (false) {}
                    else if (k == "path") path_opt = v;
                    else if (k == "file") file_opt = v;
                    else if (k == "name") name_opt = v;
                    else MSS(false, log::error() << "Unknown attribute `" << k << ":" << v << "` for tag `" << tag << "` in `" << filepath << "`" << std::endl);
                }

                MSS(!!path_opt, log::error() << "Expected filepath to be set (attribute root.path)" << std::endl);
                const auto &path = *path_opt;

                paths.push_back(path);

                if (file_opt)
                {
                    std::filesystem::path fp = *file_opt;
                    if (!fp.is_absolute())
                    {
                        fp = *path_opt;
                        fp /= *file_opt;
                    }
                    default_inputs.push_back(fp.string());
                }
                if (name_opt)
                    name__path[*name_opt] = path;
            }
            else MSS(false, log::error() << "Unknown config tag `" << tag << "` in `" << filepath << "`" << std::endl);
        }

        MSS_END();
    }

    std::string Config::substitute_names(const std::string &str) const
    {
        std::string res = str;
        for (const auto &[name, path]: name__path)
        {
            const std::string name_esc = std::string("${")+name+"}";
            while (true)
            {
                const auto ix = res.find(name_esc);
                if (ix == std::string::npos)
                    break;
                res.replace(ix, name_esc.size(), path);
            }
        }
        return res;
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
