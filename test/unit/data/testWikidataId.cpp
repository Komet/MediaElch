#include "test/test_helpers.h"

#include "data/WikidataId.h"

TEST_CASE("WikidataId", "[data]")
{
    SECTION("Empty ID")
    {
        CHECK(WikidataId().toString().isEmpty());
        CHECK(WikidataId() == WikidataId::NoId);
        CHECK_FALSE(WikidataId().isValid());
    }

    SECTION("validation / comparison")
    {
        CHECK(WikidataId("Q42").isValid());
        CHECK_FALSE(WikidataId("q42").isValid());
        CHECK_FALSE(WikidataId("Q").isValid());
        CHECK_FALSE(WikidataId("Q42x").isValid());
        CHECK(WikidataId("Q42").toString() == "Q42");
        CHECK(WikidataId("Q42") == WikidataId("Q42"));
        CHECK(WikidataId("Q42") != WikidataId("Q43"));
    }
}
