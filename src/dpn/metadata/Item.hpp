#ifndef HEADER_dpn_metadata_Item_hpp_ALREADY_INCLUDED
#define HEADER_dpn_metadata_Item_hpp_ALREADY_INCLUDED

#include <gubg/Strange.hpp>
#include <string>
#include <ostream>
#include <vector>

namespace dpn { namespace metadata { 

    class Item
    {
    public:
        std::string key;
        std::string value;

        Item() {}
        Item(const std::string &value): value(value) {}
        Item(const std::string &key, const std::string &value): key(key), value(value) {}

        bool operator==(const Item &rhs) const {return key == rhs.key && value == rhs.value;}
        bool operator!=(const Item &rhs) const {return !operator==(rhs);}
        bool operator<(const Item &rhs) const {return std::make_pair(key, value) < std::make_pair(rhs.key, rhs.value);}

        void clear() {*this = Item();}

        //Expects strange to be positioned at '@'
        bool parse(gubg::Strange &strange);

        void stream(std::ostream &os) const;

    private:
    };

    inline std::ostream &operator<<(std::ostream &os, const Item &item)
    {
        item.stream(os);
        return os;
    }

    void split(std::string &text, std::vector<Item> &items, const std::string &line);

} }

#endif
