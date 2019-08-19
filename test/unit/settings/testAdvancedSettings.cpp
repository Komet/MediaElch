#include "test/test_helpers.h"

#include "media_centers/kodi/MovieXmlReader.h"
#include "settings/AdvancedSettings.h"

#include <QString>

TEST_CASE("Advanced Settings XML", "[settings]")
{
    SECTION("empty xml")
    {
        QString emptyXml = R"xml(<?xml version="1.0" encoding="utf-8"?>
<advancedsettings>
</advancedsettings>
)xml";
        AdvancedSettings settings;
        settings.loadFromXml(emptyXml);

        AdvancedSettings defaults;
        // check a few defaults
        CHECK(settings.useFirstStudioOnly() == defaults.useFirstStudioOnly());
        CHECK(settings.forceCache() == defaults.forceCache());
        CHECK(settings.portableMode() == defaults.portableMode());
        CHECK(settings.episodeThumbnailDimensions() == defaults.episodeThumbnailDimensions());
    }

    SECTION("xml with content")
    {
        QString emptyXml = R"xml(<?xml version="1.0" encoding="utf-8"?>
<advancedsettings>
    <log>
        <debug>true</debug>
        <file>./MediaElchTest.log</file>
    </log>
    <genres>
        <map from="SciFi" to="Science Fiction" />
    </genres>
</advancedsettings>
)xml";
        AdvancedSettings settings;
        settings.loadFromXml(emptyXml);

        CHECK(settings.debugLog());
        CHECK(settings.logFile() == "./MediaElchTest.log");
        REQUIRE(settings.genreMappings().size() == 1);
        CHECK(settings.genreMappings()["SciFi"] == "Science Fiction");
    }
}
