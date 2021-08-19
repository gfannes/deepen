#ifndef HEADER_dpn_App_hpp_ALREADY_INCLUDED
#define HEADER_dpn_App_hpp_ALREADY_INCLUDED

#include <dpn/Options.hpp>
#include <dpn/config/Config.hpp>

namespace dpn { 

    class App
    {
    public:
        App(const Options &options): options_(options) {}

        bool load_config();

        bool run();

    private:
        const Options &options_;
        config::Config config_;
    };

}

#endif
