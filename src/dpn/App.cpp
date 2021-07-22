#include <dpn/App.hpp>
#include <dpn/File.hpp>
#include <gubg/mss.hpp>

namespace dpn { 

    bool App::run()
    {
        MSS_BEGIN(bool);

        MSS(!options_.input_filepath.empty());
        File file;
        MSS(file.load_from_file(options_.input_filepath));
        std::cout << file << std::endl;

        MSS_END();
    }

}
