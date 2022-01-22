#include "test/test_helpers.h"

#include "src/export/ExportTemplateLoader.h"


TEST_CASE("ExportTemplateLoader", "[export]")
{
    SECTION("parses metadata.xml")
    {
        QString metadataXml = R"xml(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
            <metadata>
                <name>MediaElch</name>
                <identifier>mediaelch-default</identifier>
                <website>https://github.com/mediaelch/mediaelch-theme-default</website>
                <description>Default Theme</description>
                <description lang="de">MediaElch Standard Theme</description>
                <author>Daniel Kabel</author>
                <author>Andre Meyering</author>
                <version>1.2.3</version>
                <mediaelch-min>2.6.8</mediaelch-min>
                <mediaelch-max>3.1.2</mediaelch-max>
                <engine>simple</engine>
                <supports>
                    <section>movies</section>
                    <section>tvshows</section>
                    <section>concerts</section>
                </supports>
            </metadata>
        )xml";

        QXmlStreamReader xml(metadataXml);
        std::unique_ptr<ExportTemplate> meta{mediaelch::exports::parseTemplate(xml, nullptr)};

        CHECK(meta->name() == "MediaElch");
        CHECK(meta->identifier() == "mediaelch-default");
        CHECK(meta->website() == "https://github.com/mediaelch/mediaelch-theme-default");
        CHECK(meta->descriptions()
              == QMap<QString, QString>{//
                  {"", "Default Theme"},
                  {"de", "MediaElch Standard Theme"}});
        CHECK(meta->authors() == QStringList{"Daniel Kabel", "Andre Meyering"});
        CHECK(meta->templateEngine() == ExportEngine::Simple);
        CHECK(meta->mediaElchVersionMin() == mediaelch::VersionInfo("2.6.8"));
        CHECK(meta->mediaElchVersionMax() == mediaelch::VersionInfo("3.1.2"));
        CHECK(meta->exportSections()
              == QVector<ExportTemplate::ExportSection>{ExportTemplate::ExportSection::Movies,
                  ExportTemplate::ExportSection::TvShows,
                  ExportTemplate::ExportSection::Concerts});
    }
}
