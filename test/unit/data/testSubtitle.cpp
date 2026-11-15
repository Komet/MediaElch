#include "test/test_helpers.h"

#include "data/Subtitle.h"

#include <QSignalSpy>

TEST_CASE("Subtitle tracks edits and emits change signals", "[data]")
{
    Subtitle subtitle;
    QSignalSpy changedSpy(&subtitle, &Subtitle::sigChanged);
    REQUIRE(changedSpy.isValid());
    CHECK(subtitle.language().isEmpty());
    CHECK(subtitle.files().isEmpty());
    CHECK_FALSE(subtitle.forced());
    CHECK_FALSE(subtitle.changed());

    SECTION("Language and forced state edits emit change signals")
    {
        subtitle.setLanguage("en");
        CHECK(subtitle.changed());
        CHECK(changedSpy.count() == 1);

        subtitle.setChanged(false);
        CHECK_FALSE(subtitle.changed());
        subtitle.setForced(true);
        CHECK(subtitle.forced());
        CHECK(subtitle.changed());
        CHECK(changedSpy.count() == 2);
    }

    SECTION("File changes can opt out of change tracking")
    {
        subtitle.setFiles({"movie.en.srt"}, false);
        CHECK(subtitle.files() == QStringList{"movie.en.srt"});
        CHECK_FALSE(subtitle.changed());
        CHECK(changedSpy.count() == 0);
        subtitle.setFiles({"movie.de.srt"});
        CHECK(subtitle.changed());
        CHECK(changedSpy.count() == 1);
    }
}
