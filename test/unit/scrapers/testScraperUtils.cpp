#include "test/test_helpers.h"

#include "scrapers/ScraperUtils.h"

using namespace mediaelch;

TEST_CASE("HTML normalization", "[scrapers]")
{
    SECTION("HTML entities")
    {
        CHECK(removeHtmlEntities("<p>Fish &amp; chips</p>") == "Fish & chips");
        CHECK(removeHtmlEntities("").isEmpty());
    }

    SECTION("Unicode whitespace")
    {
        QString noSpaces;
        removeUnicodeSpaces(noSpaces);
        CHECK(noSpaces.isEmpty());

        QString spaces = QString::fromUtf8("a\u00a0\u200b  b\t\nc");
        removeUnicodeSpaces(spaces);
        CHECK(spaces == "a b c");
    }

    SECTION("HTML normalization")
    {
        CHECK(normalizeFromHtml("<p>first\n\nsecond</p>") == "first second");
    }
}

TEST_CASE("normalizeFromHtml combines HTML and whitespace normalization", "[scrapers]")
{
    CHECK(normalizeFromHtml("<p>A&nbsp; &amp; B</p>") == "A & B");
    CHECK(normalizeFromHtml("").isEmpty());
}
