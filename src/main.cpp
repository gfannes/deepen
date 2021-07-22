#include <dpn/log.hpp>
#include <dpn/Options.hpp>
#include <dpn/App.hpp>
#include <gubg/mss.hpp>

int main(int argc, const char **argv)
{
    MSS_BEGIN(int);

    using namespace dpn;

    Options options;
    MSS(options.parse(argc, argv), log::error() << "Could not parse the CLI arguments" << std::endl);
    log::set_verbosity_level(options.verbosity_level);

    if (options.print_help)
    {
        log::os(0) << options.help();
    }
    else
    {
        App app{options};
        MSS(app.run(), log::error() << "Could not run App" << std::endl);
    }

    MSS_END();
}
