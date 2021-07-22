#ifndef HEADER_dpn_App_hpp_ALREADY_INCLUDED
#define HEADER_dpn_App_hpp_ALREADY_INCLUDED

#include <dpn/Options.hpp>

namespace dpn { 

    class App
    {
    public:
        App(const Options &options): options_(options) {}

        bool run();

    private:
        const Options &options_;
    };

}

#endif
