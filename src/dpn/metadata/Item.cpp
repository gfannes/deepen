#include <dpn/metadata/Item.hpp>
#include <gubg/mss.hpp>

namespace dpn { namespace metadata { 

    bool Item::parse(gubg::Strange &strange)
    {
        if (!strange.pop_if('@'))
            return false;

        clear();

        if (strange.pop_bracket(key, "[]"))
        {
            is_link = true;
            strange.pop_bracket(value, "()");
        }
        else
        {
            gubg::Strange content;
            strange.pop_until(content, ' ') || strange.pop_until(content, '@') || strange.pop_all(content);

            content.pop_until(key, ':');
            content.pop_all(value);
        }

        return true;
    }

    void Item::stream(std::ostream &os) const
    {
        os << '@';
        if (is_link)
        {
            os << "[" << key << "](" << value << ")";
        }
        else
        {
            if (!key.empty())
                os << key << ':';
            os << value;
        }
    }

    void split(std::string &text, std::vector<Item> &items, const std::string &line)
    {
        items.resize(0);

        if (line.empty())
        {
            text.clear();
            return;
        }

        gubg::Strange strange{line};

        if (gubg::Strange tmp; strange.front() == '@' || (strange.pop_to(tmp, " @") && strange.pop_front()))
        {
            tmp.rtrim(' ');
            tmp.pop_all(text);

            Item item;
            while (item.parse(strange))
            {
                items.push_back(item);
                strange.ltrim(' ');
            }
        }
        else
        {
            strange.pop_all(text);
        }
    }

} }
