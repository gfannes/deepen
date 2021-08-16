#include <dpn/App.hpp>
#include <dpn/input/from_file.hpp>
#include <dpn/File.hpp>
#include <gubg/mss.hpp>

namespace dpn { 

    bool App::run()
    {
        MSS_BEGIN(bool);

        MSS(!options_.input_filepath.empty());
#if 0
        File file;
        MSS(file.load_from_file(options_.input_filepath));
        std::cout << file << std::endl;
#else
        section::Sections sections;
        MSS(input::append_from_file(sections, options_.input_filepath));

        //Aggregate metadata

        for (const auto &section: sections)
            std::cout << section << std::endl;
#endif

        MSS_END();
    }

}
