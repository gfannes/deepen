#ifndef HEADER_dpn_metadata_Item_hpp_ALREADY_INCLUDED
#define HEADER_dpn_metadata_Item_hpp_ALREADY_INCLUDED

#include <gubg/Strange.hpp>
#include <string>
#include <ostream>
#include <vector>
#include <tuple>

namespace dpn { namespace metadata { 

    class Item
    {
    public:
        enum Type {User, Generated};
        Type type = User;
        std::string key;
        std::string value;
        bool is_link = false;

        Item() {}
        Item(const std::string &value): value(value) {}
        Item(const std::string &key, const std::string &value): key(key), value(value) {}
        Item(Type type, const std::string &key, const std::string &value): type(type), key(key), value(value) {}

        bool operator==(const Item &rhs) const {return type == rhs.type && key == rhs.key && value == rhs.value && is_link == rhs.is_link;}
        bool operator!=(const Item &rhs) const {return !operator==(rhs);}
        bool operator<(const Item &rhs) const {return std::make_tuple((int)type, key, value, is_link) < std::make_tuple((int)rhs.type, rhs.key, rhs.value, rhs.is_link);}

        void clear() {*this = Item();}

        //Expects strange to be positioned at '@' or '&'
        bool parse(gubg::Strange &strange);

        void stream(std::ostream &os) const;
    };

    inline std::ostream &operator<<(std::ostream &os, const Item &item)
    {
        item.stream(os);
        return os;
    }

    void split(std::string &text, std::vector<Item> &items, gubg::Strange &line);

} }

#endif
