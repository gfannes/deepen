#ifndef HEADER_dpn_input_util_hpp_ALREADY_INCLUDED
#define HEADER_dpn_input_util_hpp_ALREADY_INCLUDED

#include <dpn/log.hpp>
#include <gubg/Strange.hpp>
#include <gubg/mss.hpp>

namespace dpn { namespace input { namespace util { 

    inline bool pop_title(gubg::Strange &line, unsigned int &depth)
    {
        MSS_BEGIN(bool);

        for (depth = 0; line.pop_if('#'); ++depth) {}
        MSS_Q(depth > 0);

        MSS(line.pop_if(' '), log::warning() << "Expected a ` ` after a Title indication" << std::endl);

        MSS_END();
    }

    inline bool pop_bullet(gubg::Strange &line, unsigned int &depth)
    {
        MSS_BEGIN(bool);

        gubg::Strange orig = line;

        if (gubg::Strange prefix; line.pop_to(prefix, '*'))
        {
            depth = prefix.size();
            for (; line.pop_if('*'); ++depth) {}

            if (!line.pop_if(' '))
            {
                line = orig;
                depth = 0;
            }
        }
        else
        {
            depth = 0;
        }

        MSS_END();
    }

} } }

#endif
