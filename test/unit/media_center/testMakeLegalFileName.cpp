#include "test/test_helpers.h"

#include "media_center/kodi/MakeLegalFileName.h"

using namespace mediaelch::kodi;

TEST_CASE("kodi::makeLegalFileName", "[media_center][kodi]")
{
    SECTION("legal name is not modified")
    {
        CHECK(makeLegalFileName("Alien Collection") == "Alien Collection");
        CHECK(makeLegalFileName(".Set  Name$") == ".Set  Name$");
        CHECK(makeLegalFileName("") == "");
    }

    SECTION("colon becomes an underscore and the following space is kept")
    {
        CHECK(makeLegalFileName("Mission: Impossible Collection") == "Mission_ Impossible Collection");
    }

    SECTION("all nine illegal characters become underscores")
    {
        CHECK(makeLegalFileName("a/b") == "a_b");
        CHECK(makeLegalFileName("a\\b") == "a_b");
        CHECK(makeLegalFileName("a?b") == "a_b");
        CHECK(makeLegalFileName("a:b") == "a_b");
        CHECK(makeLegalFileName("a*b") == "a_b");
        CHECK(makeLegalFileName("a\"b") == "a_b");
        CHECK(makeLegalFileName("a<b") == "a_b");
        CHECK(makeLegalFileName("a>b") == "a_b");
        CHECK(makeLegalFileName("a|b") == "a_b");
        CHECK(makeLegalFileName(R"(/\?:*"<>|)") == "_________");
    }

    SECTION("trailing dots and spaces are trimmed, leading and interior ones are not")
    {
        CHECK(makeLegalFileName("Star Wars Collection.") == "Star Wars Collection");
        CHECK(makeLegalFileName("Star Wars Collection ") == "Star Wars Collection");
        CHECK(makeLegalFileName("Star Wars Collection. . ") == "Star Wars Collection");
        CHECK(makeLegalFileName(" .Star Wars. Collection") == " .Star Wars. Collection");
        CHECK(makeLegalFileName(". ") == "");
    }

    SECTION("an illegal character that ends up trailing is not trimmed")
    {
        CHECK(makeLegalFileName("Collection:") == "Collection_");
    }
}
