#include <dpn/App.hpp>
#include <dpn/log.hpp>
#include <dpn/input/from_file.hpp>
#include <gubg/mss.hpp>
#include <fstream>

namespace dpn { 

    bool App::run()
    {
        MSS_BEGIN(bool);

        MSS(!options_.input_filepath.empty());

        section::Sections sections;
        MSS(input::append_from_file(sections, options_.input_filepath));

        for (const auto &section: sections)
            std::cout << section << std::endl;

        if (options_.operation_opt)
        {
            switch (*options_.operation_opt)
            {
                case Operation::Update:
                    {
                        std::string output_filepath = options_.input_filepath;
                        if (!options_.output_filepath.empty())
                            output_filepath = options_.output_filepath;
                        std::ofstream fo{output_filepath};
                        section::Section::StreamConfig stream_config;
                        stream_config.mode = section::Section::StreamConfig::Original;
                        for (const auto &section: sections)
                            section.stream(fo, 0, stream_config);
                    }
                    break;
                case Operation::Export:
                    {
                        MSS(!options_.output_filepath.empty(), log::error() << "Export requires an output filepath" << std::endl;);
                        std::ofstream fo{options_.output_filepath};
                        section::Section::StreamConfig stream_config;
                        stream_config.mode = section::Section::StreamConfig::Export;
                        for (const auto &section: sections)
                            section.stream(fo, 0, stream_config);
                    }
                    break;
            }
        }

        MSS_END();
    }

}
