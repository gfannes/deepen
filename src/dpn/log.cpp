#include <dpn/log.hpp>
#include <iostream>
#include <fstream>

namespace dpn { namespace log { 

    std::ostream &error()
    {
        auto &os = std::cout;
        os << "Error: ";
        return os;
    }
    std::ostream &warning()
    {
        auto &os = std::cout;
        os << "Warning: ";
        return os;
    }

    int g_verbosity_level = 0;
    void set_verbosity_level(int verbosity_level)
    {
        g_verbosity_level = verbosity_level;
    }

    std::ostream &os(int message_importance)
    {
        if (g_verbosity_level < message_importance)
        {
            static std::ofstream s_devnull;
            return s_devnull;
        }
        return std::cout;
    }

} }
