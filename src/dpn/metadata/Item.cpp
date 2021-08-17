#include <dpn/metadata/Item.hpp>
#include <gubg/mss.hpp>

namespace dpn { namespace metadata { 

    bool Item::parse(gubg::Strange &strange)
    {
        if (!strange.pop_if('@'))
            return false;

        clear();

        gubg::Strange content;
        strange.pop_until(content, ' ') || strange.pop_until(content, '@') || strange.pop_all(content);

        content.pop_until(key, ':');
        content.pop_all(value);

        return true;
    }

    void Item::stream(std::ostream &os) const
    {
        os << '@';
        if (!key.empty())
            os << key << ':';
        os << value;
    }

    void split(std::string &text, std::vector<Item> &items, const std::string &line)
    {
        items.resize(0);

        gubg::Strange strange{line};

        if (gubg::Strange tmp; strange.pop_to(tmp, '@'))
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
