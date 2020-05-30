#include "test/test_helpers.h"

#include "data/Locale.h"

#include <QDebug>
#include <sstream>
#include <string>

using namespace mediaelch;

TEST_CASE("Locale class", "[data]")
{
    SECTION("stringify")
    {
        CHECK(Locale("de-DE").toString() == "de-DE");
        CHECK(Locale("de-CH").toString('_') == "de_CH");
        CHECK(Locale("de-AT").toString('x') == "dexAT");
    }

    SECTION("language")
    {
        CHECK(Locale("de-DE").language() == "de");
        CHECK(Locale("pt-BR").language() == "pt");
    }

    SECTION("country")
    {
        CHECK(Locale("de-AT").country() == "AT");
        CHECK(Locale("pt-BR").country() == "BR");
    }
}
