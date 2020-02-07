#include "test/test_helpers.h"

#include "data/Rating.h"

#include <sstream>
#include <string>

using namespace mediaelch;

TEST_CASE("Rating class", "[globals]")
{
    auto i = GENERATE(1., 2., 6.9);
    auto j = GENERATE(6.91, 7., 8., 9.);

    const auto r1 = Rating{100, 5.5, 0., 10., "imdb"};
    const auto r2 = Rating{90, 9.5, 0., 10., "tvdb"};

    CHECK(r2 > r1);
    CHECK(r2 >= r1);
    CHECK(r1 < r2);
    CHECK(r1 <= r2);
}
