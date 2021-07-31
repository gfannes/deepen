#include <dpn/section/Section.hpp>
#include <catch.hpp>
using namespace dpn;

TEST_CASE("section::Section tests", "[ut][section][Section]")
{
    section::Section a;
    a.title = "a";
    REQUIRE(a == a);

    SECTION("a2")
    {
        section::Section a2;
        a2.title = "a";
        REQUIRE(a == a2);
    }

    SECTION("b")
    {
        section::Section b;
        b.title = "b";
        REQUIRE(b == b);

        REQUIRE(!(a == b));
        REQUIRE(a != b);
    }
}
