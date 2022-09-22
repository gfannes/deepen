#include <dpn/config/Config.hpp>
#include <dpn/log.hpp>

#include <gubg/naft/Reader.hpp>
#include <gubg/std/filesystem.hpp>
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

        using Reader = gubg::naft::Reader;
        Reader reader{content};

        using Item = Reader::Item;
        for (Item item; reader(item); )
        {
            switch (item.what)
            {
                case Item::NodeOpen:
                if (item.text == "root")
                {
                    std::optional<std::string> path_opt, file_opt, name_opt;
                    for (; reader(item) && item.what == Item::Attribute; )
                    {
                        if (false) ;
                        else if (item.key() == "path") path_opt = item.value();
                        else if (item.key() == "file") file_opt = item.value();
                        else if (item.key() == "name") name_opt = item.value();
                        else MSS(false, log::error() << "Unknown attribute `" << item.key() << ":" << item.value() << "` for tag `" << item.text << "` in `" << filepath << "`" << std::endl);
                    }
                    MSS(item.what == Item::NodeClose);

                    MSS(!!path_opt, log::error() << "Expected filepath to be set (attribute root.path)" << std::endl);
                    const auto &path = *path_opt;

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
                else
                    MSS(false, log::error() << "Unknown config tag `" << item.text << "` in `" << filepath << "`" << std::endl);
                break;

                default:
                MSS(false);
                break;
            }
        }
        MSS(!reader.error, log::error() << "Could not read configuration file from " << filepath << ": " << *reader.error << std::endl);

        MSS_END();
    }

    bool Config::substitute_names(std::string &str) const
    {
        MSS_BEGIN(bool);

        for (const auto &[name, path]: name__path)
        {
            const std::string name_esc = std::string("${")+name+"}";
            while (true)
            {
                const auto ix = str.find(name_esc);
                if (ix == std::string::npos)
                    break;
                str.replace(ix, name_esc.size(), path);
            }
        }

        MSS(str.find("${") == std::string::npos, log::error() << "Could not substitute all names in '" << str << "'" << std::endl);

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
