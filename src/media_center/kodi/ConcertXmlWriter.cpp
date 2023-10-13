#include "media_center/kodi/ConcertXmlWriter.h"

#include "data/concert/Concert.h"
#include "globals/Helper.h"
#include "media_center/KodiXml.h"
#include "media_center/kodi/KodiXmlWriter.h"

#include <QDomDocument>

namespace {

/// \brief Export all concert fields by writing into the given QXmlStreamWriter.
class ConcertXmlStreamExport final : public mediaelch::ConcertData::Exporter
{
public:
    struct Config
    {
        bool shouldWriteThumbUrlsToNfo{false};
    };

public:
    ConcertXmlStreamExport(QXmlStreamWriter& writer, Config exportConfig) : xml{writer}, config(std::move(exportConfig))
    {
    }

    void startExport() override {}
    void endExport() override {}

    void exportDatabaseId(mediaelch::DatabaseId databaseId) override
    {
        /* ignored */
        Q_UNUSED(databaseId);
    }

    void exportMediaCenterId(int mediaCenterId) override
    {
        /* ignored */
        Q_UNUSED(mediaCenterId);
    }

    void exportTmdbId(const TmdbId& tmdbId) override
    {
        if (tmdbId.isValid()) {
            xml.writeStartElement("uniqueid");
            xml.writeAttribute("type", "tmdb");
            if (!hasIdDefault) {
                xml.writeAttribute("default", "true");
                hasIdDefault = true;
            }
            xml.writeCharacters(tmdbId.toString());
            xml.writeEndElement();
        }
    }

    void exportImdbId(const ImdbId& imdbId) override
    {
        if (imdbId.isValid()) {
            xml.writeStartElement("uniqueid");
            xml.writeAttribute("type", "imdb");
            if (!hasIdDefault) {
                xml.writeAttribute("default", "true");
                hasIdDefault = true;
            }
            xml.writeCharacters(imdbId.toString());
            xml.writeEndElement();
        }
    }

    void exportTitle(const QString& title) override
    { //
        xml.writeTextElement("title", title);
    }

    void exportOriginalTitle(const QString& originalTitle) override
    {
        xml.writeTextElement("originaltitle", originalTitle);
    }

    void exportArtist(const QString& artist) override
    { //
        xml.writeTextElement("artist", artist);
    }

    void exportAlbum(const QString& album) override
    { //
        xml.writeTextElement("album", album);
    }

    void exportOverview(const QString& overview) override
    {
        xml.writeTextElement("plot", overview);
        // xml.writeTextElement("outline", overview);
    }

    void exportRatings(const Ratings& ratings) override
    { //
        mediaelch::kodi::writeRatings(xml, ratings);
    }

    void exportUserRating(double userRating) override
    {
        xml.writeTextElement("userrating", QString::number(userRating));
    }

    void exportReleaseDate(const QDate& releaseDate) override
    {
        if (releaseDate.isValid()) {
            xml.writeTextElement("year", releaseDate.toString("yyyy"));
        }
    }

    void exportTagline(const QString& tagline) override
    { //
        xml.writeTextElement("tagline", tagline);
    }

    void exportRuntime(const std::chrono::minutes& runtime) override
    {
        using namespace std::chrono_literals;
        if (runtime > 0min) {
            xml.writeTextElement("runtime", QString::number(runtime.count()));
        }
    }

    void exportCertification(const Certification& certification) override
    {
        if (certification.isValid()) {
            xml.writeTextElement("mpaa", certification.toString());
        }
    }

    void exportGenres(const QStringList& genres) override
    { //
        KodiXml::writeStringsAsOneTagEach(xml, "genre", genres);
    }

    void exportTags(const QStringList& tags) override
    { //
        KodiXml::writeStringsAsOneTagEach(xml, "tag", tags);
    }

    void exportTrailer(const QUrl& trailer) override
    {
        if (trailer.isValid()) {
            xml.writeTextElement("trailer", helper::formatTrailerUrl(trailer.toString()));
        }
    }

    void exportPlaycount(const int& playcount) override
    {
        if (playcount > 0) {
            xml.writeTextElement("playcount", QString::number(playcount));
        }
    }

    void exportLastPlayed(const QDateTime& lastPlayed) override
    {
        if (lastPlayed.isValid()) {
            xml.writeTextElement("lastplayed", lastPlayed.toString("yyyy-MM-dd HH:mm:ss"));
        }
    }

    void exportLastModified(const QDateTime& lastModified) override
    {
        // TODO
        Q_UNUSED(lastModified);
    }

    void exportPosters(const QVector<Poster>& posters) override
    {
        if (config.shouldWriteThumbUrlsToNfo && !posters.isEmpty()) {
            for (const Poster& poster : posters) {
                if (poster.originalUrl.isValid()) {
                    xml.writeStartElement("thumb");
                    const QString aspect = poster.aspect.isEmpty() ? "poster" : poster.aspect;
                    xml.writeAttribute("aspect", aspect);
                    if (poster.thumbUrl.isValid()) {
                        xml.writeAttribute("preview", poster.thumbUrl.toString());
                    }
                    xml.writeCharacters(poster.originalUrl.toString());
                    xml.writeEndElement();
                }
            }
        }
    }

    void exportBackdrops(const QVector<Poster>& backdrops) override
    {
        if (config.shouldWriteThumbUrlsToNfo && !backdrops.isEmpty()) {
            xml.writeStartElement("fanart");
            for (const Poster& poster : backdrops) {
                if (poster.originalUrl.isValid()) {
                    xml.writeStartElement("thumb");
                    if (poster.thumbUrl.isValid()) {
                        xml.writeAttribute("preview", poster.thumbUrl.toString());
                    }
                    xml.writeCharacters(poster.originalUrl.toString());
                    xml.writeEndElement();
                }
            }
            xml.writeEndElement();
        }
    }

    void exportExtraFanarts(const QStringList& extraFanarts) override
    {
        // TODO
        Q_UNUSED(extraFanarts);
    }

    void exportStreamDetails(const StreamDetails* streamDetails) override
    {
        KodiXml::writeStreamDetails(xml, streamDetails, {});
    }

    void exportImages(const QMap<ImageType, QByteArray>& images) override
    {
        /* ignored */
        Q_UNUSED(images);
    }

    void exportFiles(const mediaelch::FileList& files) override
    {
        /* ignored */
        Q_UNUSED(files);
    }

public:
    QXmlStreamWriter& xml;
    Config config;
    bool hasIdDefault{false};
};

} // namespace

namespace mediaelch {
namespace kodi {

ConcertXmlWriterGeneric::ConcertXmlWriterGeneric(KodiVersion version, const Concert& concert) :
    ConcertXmlWriter(std::move(version)), m_concert{concert}
{
}

QByteArray ConcertXmlWriterGeneric::getConcertXml(bool testMode)
{
    QByteArray xmlContent;
    QXmlStreamWriter xml(&xmlContent);

    ConcertXmlStreamExport::Config config{};
    config.shouldWriteThumbUrlsToNfo = writeThumbUrlsToNfo();

    xml.setAutoFormatting(true);
    xml.writeStartDocument("1.0", true);
    xml.writeStartElement("musicvideo");

    ConcertXmlStreamExport exporter(xml, config);
    m_concert.exportTo(exporter);

    if (!testMode) {
        addMediaelchGeneratorTag(xml);
    }
    xml.writeEndElement();
    xml.writeEndDocument();
    return xmlContent;
}

} // namespace kodi
} // namespace mediaelch
