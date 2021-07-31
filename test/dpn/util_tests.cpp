#include <dpn/util.hpp>
#include <catch.hpp>
using namespace dpn;

TEST_CASE("util::lead_count() tests", "[ut][util][lead_count]")
{
    REQUIRE(util::lead_count('#', ' ', "")        == std::make_pair(0u, 0u));
    REQUIRE(util::lead_count('#', ' ', "#")       == std::make_pair(1u, 0u));
    REQUIRE(util::lead_count('#', ' ', "# ")      == std::make_pair(1u, 1u));
    REQUIRE(util::lead_count('#', ' ', "##")      == std::make_pair(2u, 0u));
    REQUIRE(util::lead_count('#', ' ', "## ")     == std::make_pair(2u, 1u));
    REQUIRE(util::lead_count('#', ' ', "##  ")    == std::make_pair(2u, 2u));
    REQUIRE(util::lead_count('#', ' ', "##  abc") == std::make_pair(2u, 2u));
}
