#include <dpn/input/from_file.hpp>
#include <catch.hpp>
using namespace dpn;

#if 0
TEST_CASE("input/from_file tests", "[disabled][input][from_file]")
{
    struct Scn
    {
        std::string content;
        std::string filepath = "filepath.md";
    };
    struct Exp
    {
        bool ok = true;
        section::Sections sections;
    };

    Scn scn;
    Exp exp;

    SECTION("empty") { }
    SECTION("a")
    {
        scn.content = R"eod(# a
line0
line1)eod";
        
        section::Section a;
        a.text = "a";
        a.lines.push_back("line0");
        a.lines.push_back("line1");
        a.filepath = "filepath.md";
        exp.sections.push_back(a);
    }
    SECTION("a b c")
    {
        scn.content = R"eod(# a
#   b
# c)eod";

        section::Section a; a.text = "a";
        a.filepath = "filepath.md";
        exp.sections.push_back(a);
        section::Section b; b.text = "b";
        b.filepath = "filepath.md";
        exp.sections.push_back(b);
        section::Section c; c.text = "c";
        c.filepath = "filepath.md";
        exp.sections.push_back(c);
    }
    SECTION("a aa b")
    {
        scn.content = R"eod(# a
## aa
# b)eod";

        section::Section a; a.text = "a";
        a.filepath = "filepath.md";
        section::Section aa; aa.text = "aa";
        a.childs.push_back(aa);
        exp.sections.push_back(a);

        section::Section b; b.text = "b";
        b.filepath = "filepath.md";
        exp.sections.push_back(b);
    }
    SECTION("a aaa b")
    {
        scn.content = R"eod(# a
### aaa
#### aaaa
### aaa2
# b)eod";

        section::Section a; a.text = "a";
        a.filepath = "filepath.md";
        section::Section aaa; aaa.text = "aaa";
        section::Section aaaa; aaaa.text = "aaaa";
        aaa.childs.push_back(aaaa);
        a.childs.push_back(aaa);
        section::Section aaa2; aaa2.text = "aaa2";
        a.childs.push_back(aaa2);
        exp.sections.push_back(a);

        section::Section b; b.text = "b";
        b.filepath = "filepath.md";
        exp.sections.push_back(b);
    }

    section::Sections sections;
    const auto ok = input::append_from_string(sections, scn.content, scn.filepath);
    REQUIRE(ok == exp.ok);
    if (ok)
    {
        REQUIRE(sections == exp.sections);
    }
}
#endif
