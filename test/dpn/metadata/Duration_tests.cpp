#include <dpn/metadata/Duration.hpp>
#include <catch.hpp>
#include <sstream>
using namespace dpn;

TEST_CASE("metadata::Duration tests", "[ut][metadata][Duration]")
{
    metadata::Duration duration;
    REQUIRE(duration.minutes == 0.0);

    duration.minutes = 1;
    const auto copy = duration;
    REQUIRE(copy.minutes == 1.0);

    duration += copy;
    duration += copy;
    duration += copy;
    REQUIRE(duration.minutes == 4.0);
}

TEST_CASE("metadata::Duration.parse tests", "[ut][metadata][Duration][parse]")
{
    struct Scn
    {
        std::string content;
    };
    struct Exp
    {
        bool parse_ok = true;
        double minutes = 0.0;
    };

    Scn scn;
    Exp exp;

    SECTION("positive")
    {
        SECTION("0m")
        {
            scn.content = "0m";
            exp.minutes = 0.0;
        }
        SECTION("2m")
        {
            scn.content = "2m";
            exp.minutes = 2.0;
        }
        SECTION("2h")
        {
            scn.content = "2h";
            exp.minutes = 120.0;
        }
        SECTION("2d")
        {
            scn.content = "2d";
            exp.minutes = 960.0;
        }
        SECTION("2w")
        {
            scn.content = "2w";
            exp.minutes = 4800.0;
        }
        SECTION("2w2d2h2m")
        {
            scn.content = "2w2d2h2m";
            exp.minutes = 4800.0+960.0+120.0+2.0;
        }
    }
    SECTION("negative")
    {
        exp.parse_ok = false;
        SECTION("default") { }
        SECTION("2") { scn.content = "2"; }
    }

    metadata::Duration duration;
    gubg::Strange strange{scn.content};
    const auto parse_ok = duration.parse(strange);
    REQUIRE(parse_ok == exp.parse_ok);
    if (parse_ok)
    {
        REQUIRE(duration.minutes == exp.minutes);

        std::ostringstream oss;
        oss << duration;
        REQUIRE(oss.str() == scn.content);
    }
    else
    {
        REQUIRE(strange.str() == scn.content);
    }
}
