#include <dpn/metadata/Item.hpp>
#include <catch.hpp>
#include <sstream>
using namespace dpn;

TEST_CASE("metadata::Item tests", "[ut][metadata][Item]")
{
    metadata::Item item;
    REQUIRE(item == item);
    REQUIRE(!(item != item));
    SECTION("value")
    {
        const metadata::Item v{"value"};
        item.value = "value";
        REQUIRE(item == item);
    }
    SECTION("key value")
    {
        const metadata::Item kv{"key", "value"};
        item.key = "key";
        item.value = "value";
        REQUIRE(item == item);
    }
}

TEST_CASE("metadata::Item.parse tests", "[ut][metadata][Item][parse]")
{
    struct Scn
    {
        std::string str;
    };
    struct Exp
    {
        bool parse_ok = false;
        metadata::Item item;
        bool serialized_matches = true;
    };

    Scn scn;
    Exp exp;

    SECTION("positive")
    {
        exp.parse_ok = true;
        SECTION("@value")
        {
            scn.str = "@value";
            exp.item.value = "value";
        }
        SECTION("@key:value")
        {
            scn.str = "@key:value";
            exp.item.key = "key";
            exp.item.value = "value";
        }
        SECTION("with extra items")
        {
            exp.serialized_matches = false;
            SECTION("@value extra")
            {
                scn.str = "@value extra";
                exp.item.value = "value";
            }
            SECTION("@value@value2")
            {
                scn.str = "@value@value2";
                exp.item.value = "value";
            }
        }
    }
    SECTION("negative")
    {
        exp.parse_ok = false;
        SECTION("default") { }
        SECTION("no_@")
        {
            scn.str = "no_@";
        }
    }

    gubg::Strange strange{scn.str};
    metadata::Item item;
    const auto parse_ok = item.parse(strange);
    REQUIRE(parse_ok == exp.parse_ok);
    if (parse_ok)
    {
        REQUIRE(item == exp.item);

        //Check that serialized output matches the input
        std::ostringstream oss;
        oss << item;
        const auto serialized_matches = oss.str() == scn.str;
        REQUIRE(serialized_matches == exp.serialized_matches);
    }
    else
    {
        REQUIRE(strange.str() == scn.str);
    }
}

TEST_CASE("metadata::Item::split() tests", "[ut][metadata][Item][split]")
{
    struct Scn
    {
        std::string line;
    };
    struct Exp
    {
        std::string text;
        std::vector<metadata::Item> items;
    };

    Scn scn;
    Exp exp;

    SECTION("default") { }
    SECTION("text @md1")
    {
        scn.line = "text @md1";
        exp.text = "text";
        exp.items.push_back(metadata::Item{"md1"});
    }
    SECTION("text @md1 @md2")
    {
        scn.line = "text @md1 @md2";
        exp.text = "text";
        exp.items.push_back(metadata::Item{"md1"});
        exp.items.push_back(metadata::Item{"md2"});
    }

    std::string text;
    std::vector<metadata::Item> items;
    metadata::split(text, items, scn.line);
    REQUIRE(text == exp.text);
    REQUIRE(items == exp.items);
}
