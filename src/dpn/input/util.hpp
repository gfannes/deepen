#ifndef HEADER_dpn_input_util_hpp_ALREADY_INCLUDED
#define HEADER_dpn_input_util_hpp_ALREADY_INCLUDED

#include <dpn/log.hpp>
#include <gubg/Strange.hpp>
#include <gubg/mss.hpp>

namespace dpn { namespace input { namespace util { 

    inline bool pop_title(gubg::Strange &line, unsigned int &depth)
    {
        MSS_BEGIN(bool);

        auto copy = line;
        auto revert = [&](){line = copy;};

        depth = 0;

        if (false) {}
        else if (line.pop_if('#'))//Markdown
        {
            for (depth = 1; line.pop_if('#'); ++depth) {}
        }
        else if (line.pop_if('h'))//Textile/JIRA
        {
            MSS_Q(line.pop_decimal(depth), revert());
            MSS_Q(line.pop_if('.'), revert());
        }

        MSS_Q(depth > 0, revert());

        MSS(line.pop_if(' '), (revert(), log::warning() << "Expected a ` ` after a Title indication" << std::endl));

        MSS_END();
    }

    inline bool pop_bullet(gubg::Strange &line, unsigned int &depth)
    {
        MSS_BEGIN(bool);

        gubg::Strange orig = line;

        if (gubg::Strange prefix; line.pop_to(prefix, '*'))//Markdown/Textile/JIRA
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
