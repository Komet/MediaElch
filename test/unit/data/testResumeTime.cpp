#include "test/test_helpers.h"

#include "data/ResumeTime.h"

#include <QDebug>

using namespace mediaelch;

TEST_CASE("ResumeTime", "[data]")
{
    SECTION("default values are zero")
    {
        const ResumeTime t;
        CHECK(t.position == 0.0);
        CHECK(t.total == 0.0);
    }

    SECTION("constructor arguments")
    {
        const ResumeTime t{42.5, 120.0};
        CHECK(t.position == 42.5);
        CHECK(t.total == 120.0);
    }

    SECTION("QDebug operator produces expected output")
    {
        const ResumeTime t{1.5, 90.0};
        QString buffer;
        QDebug stream(&buffer);
        stream << t;
        CHECK(buffer.contains("position=1.5"));
        CHECK(buffer.contains("total=90"));
    }
}
