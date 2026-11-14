#include "test/test_helpers.h"

#include "data/AllMusicId.h"
#include "data/MusicBrainzId.h"
#include "data/TheAudioDbId.h"

TEST_CASE("Music IDs validation", "[data][music]")
{
    SECTION("AllMusicId")
    {
        CHECK(AllMusicId().toString().isEmpty());
        CHECK(AllMusicId() == AllMusicId::NoId);
        CHECK_FALSE(AllMusicId().isValid());
        CHECK(AllMusicId("anything").isValid());
        CHECK(AllMusicId("anything").toString() == "anything");
        CHECK(AllMusicId("anything") == AllMusicId("anything"));
        CHECK(AllMusicId("anything") != AllMusicId("other"));
    }

    SECTION("MusicBrainzId")
    {
        CHECK(MusicBrainzId().toString().isEmpty());
        CHECK(MusicBrainzId() == MusicBrainzId::NoId);
        CHECK_FALSE(MusicBrainzId().isValid());
        CHECK(MusicBrainzId("anything").isValid());
        CHECK(MusicBrainzId("anything").toString() == "anything");
        CHECK(MusicBrainzId("anything") == MusicBrainzId("anything"));
        CHECK(MusicBrainzId("anything") != MusicBrainzId("other"));
    }

    SECTION("TheAudioDbId")
    {
        CHECK(TheAudioDbId().toString().isEmpty());
        CHECK(TheAudioDbId() == TheAudioDbId::NoId);
        CHECK_FALSE(TheAudioDbId().isValid());
        CHECK(TheAudioDbId("anything").isValid());
        CHECK(TheAudioDbId("anything").toString() == "anything");
        CHECK(TheAudioDbId("anything") == TheAudioDbId("anything"));
        CHECK(TheAudioDbId("anything") != TheAudioDbId("other"));
    }
}
