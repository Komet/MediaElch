#include "test/test_helpers.h"

#include "media_centers/kodi/MovieXmlReader.h"
#include "settings/AdvancedSettings.h"

#include <QString>

static QString addBaseXml(QString xml)
{
    QString fullXml = R"xml(<?xml version="1.0" encoding="utf-8"?>
<advancedsettings>
)xml";
    fullXml += xml;
    fullXml += "\n</advancedsettings>\n";
    return fullXml;
}

TEST_CASE("Advanced Settings XML", "[settings]")
{
    SECTION("empty xml")
    {
        AdvancedSettings settings;
        settings.loadFromXml(addBaseXml(""));

        AdvancedSettings defaults;
        // check a few defaults
        CHECK(settings.useFirstStudioOnly() == defaults.useFirstStudioOnly());
        CHECK(settings.forceCache() == defaults.forceCache());
        CHECK(settings.portableMode() == defaults.portableMode());
        CHECK(settings.episodeThumbnailDimensions() == defaults.episodeThumbnailDimensions());
    }

    SECTION("xml with content")
    {
        QString emptyXml = addBaseXml(R"xml(
            <log>
                <debug>true</debug>
                <file>./MediaElchTest.log</file>
            </log>
            <genres>
                <map from="SciFi" to="Science Fiction" />
            </genres>
        )xml");
        AdvancedSettings settings;
        settings.loadFromXml(emptyXml);

        CHECK(settings.debugLog());
        CHECK(settings.logFile() == "./MediaElchTest.log");
        REQUIRE(settings.genreMappings().size() == 1);
        CHECK(settings.genreMappings()["SciFi"] == "Science Fiction");
    }

    SECTION("xml with invalid content: episodeThumb")
    {
        SECTION("negative values")
        {
            QString emptyXml = addBaseXml(R"xml(
                <episodeThumb>
                   <width>-0</width>
                   <height>-100</height>
                </episodeThumb>
            )xml");

            AdvancedSettings settings;
            settings.loadFromXml(emptyXml);

            CHECK(settings.episodeThumbnailDimensions().width == 400);
            CHECK(settings.episodeThumbnailDimensions().height == 300);
        }

        SECTION("too small value values")
        {
            QString emptyXml = addBaseXml(R"xml(
                <episodeThumb>
                   <width>0</width>
                   <height>66</height>
                </episodeThumb>
            )xml");

            AdvancedSettings settings;
            settings.loadFromXml(emptyXml);

            CHECK(settings.episodeThumbnailDimensions().width == 400);
            CHECK(settings.episodeThumbnailDimensions().height == 300);
        }

        SECTION("non-integer values")
        {
            QString emptyXml = addBaseXml(R"xml(
                <episodeThumb>
                   <width>-abc</width>
                   <height>0.23</height>
                </episodeThumb>
            )xml");

            AdvancedSettings settings;
            settings.loadFromXml(emptyXml);

            CHECK(settings.episodeThumbnailDimensions().width == 400);
            CHECK(settings.episodeThumbnailDimensions().height == 300);
        }
    }
}
