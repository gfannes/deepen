#include <dpn/metadata/Status.hpp>
#include <catch.hpp>
#include <sstream>
#include <cctype>
using namespace dpn;

TEST_CASE("metadata::Status tests", "[ut][metadata][Status]")
{
    using namespace metadata;

    Status a, b;
    Status m;

    SECTION("r r")
    {
        a = Status{State::Requirement, false};
        b = Status{State::Requirement, false};
        m = Status{State::Requirement, false};
    }
    SECTION("r R")
    {
        a = Status{State::Requirement, false};
        b = Status{State::Requirement, true};
        m = Status{State::Requirement, false};
    }
    SECTION("R r")
    {
        a = Status{State::Requirement, true};
        b = Status{State::Requirement, false};
        m = Status{State::Requirement, false};
    }
    SECTION("R d")
    {
        a = Status{State::Requirement, true};
        b = Status{State::Design, false};
        m = Status{State::Requirement, true};
    }
    SECTION("r d")
    {
        a = Status{State::Requirement, false};
        b = Status{State::Design, false};
        m = Status{State::Requirement, false};
    }

    REQUIRE(metadata::Status::minimum(a, b) == m);
}

TEST_CASE("metadata::Status::parse tests", "[ut][metadata][Status][parse]")
{
    struct Scn
    {
        std::string content;
    };
    struct Exp
    {
        bool parse_ok = false;
        metadata::State state = metadata::State::Requirement;
        bool done = false;
        double fraction_done =  0.0;
    };

    Scn scn;
    Exp exp;

    SECTION("positive")
    {
        exp.parse_ok = true;
        SECTION("Requirement")
        {
            exp.state = metadata::State::Requirement;
            exp.fraction_done = 0.0;
            SECTION("wip")  { scn.content = "r"; }
            SECTION("done") { scn.content = "R"; }
        }
        SECTION("Design")
        {
            exp.state = metadata::State::Design;
            exp.fraction_done = 0.2;
            SECTION("wip")  { scn.content = "d"; }
            SECTION("done") { scn.content = "D"; }
        }
        SECTION("Starting")
        {
            exp.state = metadata::State::Starting;
            exp.fraction_done = 0.4;
            SECTION("wip")  { scn.content = "s"; }
            SECTION("done") { scn.content = "S"; }
        }
        SECTION("Implementation")
        {
            exp.state = metadata::State::Implementation;
            exp.fraction_done = 0.6;
            SECTION("wip")  { scn.content = "i"; }
            SECTION("done") { scn.content = "I"; }
        }
        SECTION("Validation")
        {
            exp.state = metadata::State::Validation;
            exp.fraction_done = 0.8;
            SECTION("wip")  { scn.content = "v"; }
            SECTION("done") { scn.content = "V"; }
        }

        if (scn.content.size() > 0)
            exp.done = !std::islower(scn.content[0]);
        if (exp.done)
            exp.fraction_done += 0.2;
    }
    SECTION("negative")
    {
        SECTION("empty") { }
        SECTION("?") { scn.content = "?"; }
    }

    gubg::Strange strange{scn.content};
    metadata::Status status;
    const auto parse_ok = status.parse(strange);
    REQUIRE(parse_ok == exp.parse_ok);
    if (parse_ok)
    {
        REQUIRE(status.done == exp.done);

        REQUIRE(status.fraction_done() == Approx(exp.fraction_done));

        std::ostringstream oss;
        oss << status;
        REQUIRE(oss.str() == scn.content);
    }
    else
    {
        REQUIRE(strange.str() == scn.content);
    }
}
