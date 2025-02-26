#include "test/test_helpers.h"

#include "globals/VersionInfo.h"

using namespace mediaelch;

TEST_CASE("version info class", "[globals]")
{
    SECTION("string")
    {
        CHECK(VersionInfo("2").toString() == "2.0.0");
        CHECK(VersionInfo("2.6").toString() == "2.6.0");
        CHECK(VersionInfo("2.6.2").toString() == "2.6.2");

        CHECK(VersionInfo("invalid").toString() == "");
    }

    SECTION("operators")
    {
        CHECK(VersionInfo("2.6.2") == VersionInfo("2.6.2"));

        CHECK(VersionInfo("3.4.0") > VersionInfo("2.6.0"));
        CHECK(VersionInfo("2.6.2") > VersionInfo("2.6.0"));
        CHECK(VersionInfo("2.8.2") > VersionInfo("2.6.0"));
        CHECK_FALSE(VersionInfo("2.6.0") > VersionInfo("2.6.0"));

        CHECK(VersionInfo("1.8.2") < VersionInfo("2.6.2"));
        CHECK(VersionInfo("2.4.2") < VersionInfo("2.6.2"));
        CHECK(VersionInfo("2.6.1") < VersionInfo("2.6.2"));
        CHECK_FALSE(VersionInfo("2.6.1") < VersionInfo("2.6.1"));

        CHECK(VersionInfo("2.6.2") >= VersionInfo("2.6.0"));
        CHECK(VersionInfo("2.6.0") >= VersionInfo("2.6.0"));

        CHECK(VersionInfo("2.6.0") != VersionInfo("2.6.2"));

        CHECK(VersionInfo("2.6.2") <= VersionInfo("2.6.3"));
        CHECK(VersionInfo("2.6.2") <= VersionInfo("2.6.2"));
    }

    SECTION("validator")
    {
        CHECK_FALSE(VersionInfo().isValid());
        CHECK_FALSE(VersionInfo("0").isValid());
        CHECK_FALSE(VersionInfo("0.0.0").isValid());
        CHECK_FALSE(VersionInfo("v2.6.2").isValid());
        CHECK_FALSE(VersionInfo("invalid").isValid());

        CHECK(VersionInfo("2").isValid());
        CHECK(VersionInfo("2.6").isValid());
        CHECK(VersionInfo("2.6.2").isValid());
    }

    SECTION("operators for invalid versions")
    {
        CHECK_FALSE(VersionInfo() > VersionInfo("2.6.2"));
        CHECK_FALSE(VersionInfo() < VersionInfo());
        CHECK_FALSE(VersionInfo("2.6.2") == VersionInfo());
        CHECK_FALSE(VersionInfo() <= VersionInfo());
        CHECK_FALSE(VersionInfo() >= VersionInfo("2.6.2"));
        // always true for invalid versions
        CHECK(VersionInfo() != VersionInfo("2.6.2"));
        CHECK(VersionInfo("2.6.2") != VersionInfo());
    }

    SECTION("stable/unstable")
    {
        CHECK(VersionInfo("2.6").isStable());
        CHECK(VersionInfo("2.6.2").isStable());
        CHECK(VersionInfo("2.6.4").isStable());

        CHECK(VersionInfo("2.6.3").isUnstable());
        CHECK(VersionInfo("2.4.3").isUnstable());

        // stable/unstable version numbering introduced with v2.4.3
        // all older versions are always stable
        CHECK(VersionInfo("2.4.1").isStable());
        CHECK(VersionInfo("1.3.1").isStable());
        CHECK(VersionInfo("2.4.2").isStable());
        CHECK(VersionInfo("2.4").isStable());
        CHECK(VersionInfo("2").isStable());

        CHECK(VersionInfo("0.0.0").isUnstable());
        CHECK(VersionInfo("invalid").isUnstable());
    }
}
