#ifndef HEADER_dpn_input_util_hpp_ALREADY_INCLUDED
#define HEADER_dpn_input_util_hpp_ALREADY_INCLUDED

#include <dpn/onto/enums.hpp>
#include <dpn/log.hpp>
#include <gubg/Strange.hpp>
#include <gubg/mss.hpp>

namespace dpn { namespace input { namespace util { 

    inline bool pop_title(gubg::Strange &line, unsigned int &depth, onto::Format &format)
    {
        MSS_BEGIN(bool);

        auto copy = line;
        auto revert = [&](){line = copy;};

        depth = 0;

        if (false) {}
        else if (line.pop_if('#'))
        {
            format = onto::Format::Markdown;
            for (depth = 1; line.pop_if('#'); ++depth) {}
        }
        else if (line.pop_if('h'))
        {
            format = onto::Format::JIRA;
            MSS_Q(line.pop_decimal(depth), revert());
            MSS_Q(line.pop_if('.'), revert());
        }

        MSS_Q(depth > 0, revert());

        MSS(line.pop_if(' '), (revert(), log::warning() << "Expected a ` ` after a Title indication" << std::endl));

        MSS_END();
    }

    inline bool pop_bullet(gubg::Strange &line, unsigned int &depth, onto::Format &format)
    {
        MSS_BEGIN(bool);

        gubg::Strange orig = line;

        if (gubg::Strange prefix; line.pop_to(prefix, '*'))//Markdown/Textile/JIRA
        {
            depth = prefix.size();
            format = (depth == 0 ? onto::Format::JIRA : onto::Format::Markdown);
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

    inline bool pop_code_block_marker(gubg::Strange &line, onto::Format &format)
    {
        MSS_BEGIN(bool);

        auto copy = line;
        auto revert = [&](){line = copy;};

        auto is_markdown_code_block = [&](){
            const auto b = line.pop_if("```");
            if (b)
                format = onto::Format::Markdown;
            return b;
        };
        auto is_jira_code_block = [&](){
            gubg::Strange drop;
            const auto b = line.pop_if("{code") && (line.pop_if(':') ? line.pop_until(drop, '}') : line.pop_if('}'));
            if (b)
                format = onto::Format::JIRA;
            return b;
        };

        MSS_Q(is_markdown_code_block() || is_jira_code_block(), revert());

        MSS_END();
    }

} } }

#endif
