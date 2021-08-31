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

    void split(std::string &text, std::vector<Item> &items, gubg::Strange &line)
    {
        items.resize(0);

        if (line.empty())
        {
            text.clear();
            return;
        }

        if (gubg::Strange tmp; line.front() == '@' || (line.pop_to(tmp, " @") && line.pop_front()))
        {
            tmp.rtrim(' ');
            tmp.pop_all(text);

            Item item;
            while (item.parse(line))
            {
                items.push_back(item);
                line.ltrim(' ');
            }
        }
        else
        {
            line.pop_all(text);
        }
    }

} }
