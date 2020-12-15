#include "test/test_helpers.h"

#include "media_centers/kodi/MovieXmlReader.h"
#include "settings/AdvancedSettingsXmlReader.h"

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
        auto pair = AdvancedSettingsXmlReader::loadFromXml(addBaseXml(""));
        auto settings = pair.first;
        auto messages = pair.second;

        AdvancedSettings defaults;
        // check a few defaults
        CHECK(settings.useFirstStudioOnly() == defaults.useFirstStudioOnly());
        CHECK(settings.forceCache() == defaults.forceCache());
        CHECK(settings.portableMode() == defaults.portableMode());
        CHECK(settings.episodeThumbnailDimensions() == defaults.episodeThumbnailDimensions());
        CHECK(messages.isEmpty());
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

        AdvancedSettings settings = AdvancedSettingsXmlReader::loadFromXml(emptyXml).first;

        CHECK(settings.debugLog());
        CHECK(settings.logFile() == "./MediaElchTest.log");
        REQUIRE(settings.genreMappings().size() == 1);
        CHECK(settings.genreMappings()["SciFi"] == "Science Fiction");
    }

    const auto checkEpisodeThumbValues = [](const auto& pair) {
        const auto settings = pair.first;
        const auto messages = pair.second;

        CHECK(settings.episodeThumbnailDimensions().width == 400);
        CHECK(settings.episodeThumbnailDimensions().height == 300);
        REQUIRE(messages.size() == 2);
        CHECK(messages[0].type == AdvancedSettingsXmlReader::ParseErrorType::InvalidValue);
        CHECK(messages[0].tag == "width");
        CHECK(messages[1].type == AdvancedSettingsXmlReader::ParseErrorType::InvalidValue);
        CHECK(messages[1].tag == "height");
    };

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

            checkEpisodeThumbValues(AdvancedSettingsXmlReader::loadFromXml(emptyXml));
        }

        SECTION("too small value values")
        {
            QString emptyXml = addBaseXml(R"xml(
                <episodeThumb>
                    <width>0</width>
                    <height>66</height>
                </episodeThumb>
            )xml");

            checkEpisodeThumbValues(AdvancedSettingsXmlReader::loadFromXml(emptyXml));
        }

        SECTION("non-integer values")
        {
            QString emptyXml = addBaseXml(R"xml(
                <episodeThumb>
                    <width>-abc</width>
                    <height>0.23</height>
                </episodeThumb>
            )xml");

            checkEpisodeThumbValues(AdvancedSettingsXmlReader::loadFromXml(emptyXml));
        }

        SECTION("unknown tags")
        {
            QString emptyXml = addBaseXml(R"xml(
                <episodeThumb>
                    <witdh>100</witdh>
                    <heihgt>200</heihgt>
                </episodeThumb>
                <unknown>something</unknown>
            )xml");

            const auto messages = AdvancedSettingsXmlReader::loadFromXml(emptyXml).second;

            REQUIRE(messages.size() == 3);
            CHECK(messages[0].tag == "witdh");
            CHECK(messages[1].tag == "heihgt");
            CHECK(messages[2].tag == "unknown");
        }
    }

    SECTION("exclude patterns")
    {
        SECTION("invalid attribute value")
        {
            QString xml = addBaseXml(R"xml(
                <exclude>
                    <pattern applyTo="invalid" />
                </exclude>
            )xml");

            const auto messages = AdvancedSettingsXmlReader::loadFromXml(xml).second;

            REQUIRE(messages.size() == 1);
            CHECK(messages[0].tag == "pattern");
            CHECK(messages[0].type == AdvancedSettingsXmlReader::ParseErrorType::InvalidAttributeValue);
        }
    }

    SECTION("read attributes correctly")
    {
        QString xml = addBaseXml(R"xml(
            <studios useFirstStudioOnly="true"/>
        )xml");

        const auto pair = AdvancedSettingsXmlReader::loadFromXml(xml);
        const auto settings = pair.first;
        const auto messages = pair.second;

        REQUIRE(messages.empty());
        CHECK(settings.useFirstStudioOnly() == true);
    }
}
