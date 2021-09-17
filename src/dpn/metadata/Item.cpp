#include <dpn/metadata/Item.hpp>
#include <gubg/mss.hpp>

namespace dpn { namespace metadata { 

    bool Item::parse(gubg::Strange &strange)
    {
        if (strange.empty())
            return false;

        switch (strange.front())
        {
            case '@': clear(); type = User; break;
            case '&': clear(); type = Generated; break;
            default: return false; break;
        }
        strange.pop_count(1);

        if (strange.pop_bracket(key, "[]"))
        {
            is_link = true;
            strange.pop_bracket(value, "()");
        }
        else
        {
            gubg::Strange content;
            strange.pop_until(content, ' ') || strange.pop_until(content, '@') || strange.pop_until(content, '&') || strange.pop_all(content);

            content.pop_until(key, ':');
            content.pop_all(value);
        }

        return true;
    }

    void Item::stream(std::ostream &os) const
    {
        switch (type)
        {
            case User: os << '@'; break;
            case Generated: os << '&'; break;
        }

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

        auto is_start_of_metadata = [&](){
            if (line.empty())
                return false;
            return line.front() == '@' || line.front() == '&';
        };
        auto parse_metadata_items = [&](){
            Item item;
            while (item.parse(line))
            {
                items.push_back(item);
                line.ltrim(' ');
            }
        };

        text.clear();
        if (is_start_of_metadata())
        {
            parse_metadata_items();
            return;
        }

        for (gubg::Strange part; line.pop_until(part, ' '); )
        {
            text += part.str();
            if (is_start_of_metadata())
            {
                parse_metadata_items();
                return;
            }
            text.push_back(' ');
        }
        text += line.str();
    }

} }
