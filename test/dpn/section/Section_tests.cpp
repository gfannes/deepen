#include <dpn/section/Section.hpp>
#include <catch.hpp>
using namespace dpn;

TEST_CASE("section::Section tests", "[ut][section][Section]")
{
    section::Section a;
    a.text = "a";
    REQUIRE(a == a);

    SECTION("a2")
    {
        section::Section a2;
        a2.text = "a";
        REQUIRE(a == a2);
    }

    SECTION("b")
    {
        section::Section b;
        b.text = "b";
        REQUIRE(b == b);

        REQUIRE(!(a == b));
        REQUIRE(a != b);
    }
}
