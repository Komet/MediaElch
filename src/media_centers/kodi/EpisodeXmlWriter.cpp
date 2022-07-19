#include "media_centers/kodi/EpisodeXmlWriter.h"

#include "globals/Helper.h"
#include "media_centers/KodiXml.h"
#include "media_centers/kodi/EpisodeXmlReader.h"
#include "media_centers/kodi/KodiXmlWriter.h"
#include "tv_shows/TvShowEpisode.h"

#include <QDomDocument>
#include <utility>

namespace mediaelch {
namespace kodi {

QString EpisodeXmlWriter::getEpisodeXmlWithSingleRoot(bool testMode)
{
    return EpisodeXmlReader::makeValidEpisodeXml(getEpisodeXml(testMode));
}

EpisodeXmlWriterGeneric::EpisodeXmlWriterGeneric(KodiVersion version, QVector<TvShowEpisode*> episodes) :
    EpisodeXmlWriter(std::move(version)), m_episodes{std::move(episodes)}
{
}

QByteArray EpisodeXmlWriterGeneric::getEpisodeXml(bool testMode)
{
    QByteArray xmlContent;
    QXmlStreamWriter xml(&xmlContent);
    xml.setAutoFormatting(true);
    xml.writeStartDocument("1.0", true);

    for (TvShowEpisode* subEpisode : m_episodes) {
        writeSingleEpisodeDetails(xml, subEpisode, testMode);
    }

    xml.writeEndDocument();
    return xmlContent;
}


/// \brief Writes TV show episode elements to an xml stream
/// \param xml XML stream
/// \param episode Episode to save
void EpisodeXmlWriterGeneric::writeSingleEpisodeDetails(QXmlStreamWriter& xml, TvShowEpisode* episode, bool testMode)
{
    xml.writeStartElement("episodedetails");
    xml.writeTextElement("title", episode->title());
    xml.writeTextElement("showtitle", episode->showTitle());

    // unique id: IMDb and TMDb
    // TODO: one uniqueid is required; we don't check that at the moment
    // Empty default unique ID is not valid in Kodi.
    if (episode->tvdbId().isValid()) {
        xml.writeStartElement("uniqueid");
        xml.writeAttribute("type", "tvdb");
        xml.writeAttribute("default", "true");
        xml.writeCharacters(episode->tvdbId().toString());
        xml.writeEndElement();
    }
    if (episode->imdbId().isValid()) {
        xml.writeStartElement("uniqueid");
        xml.writeAttribute("type", "imdb");
        xml.writeCharacters(episode->imdbId().toString());
        xml.writeEndElement();
    }
    if (episode->tmdbId().isValid()) {
        xml.writeStartElement("uniqueid");
        xml.writeAttribute("type", "tmdb");
        xml.writeCharacters(episode->tmdbId().toString());
        xml.writeEndElement();
    }
    if (episode->tvmazeId().isValid()) {
        xml.writeStartElement("uniqueid");
        xml.writeAttribute("type", "tvmaze");
        xml.writeCharacters(episode->tvmazeId().toString());
        xml.writeEndElement();
    }

    writeRatings(xml, episode->ratings());

    xml.writeTextElement("userrating", QString::number(episode->userRating()));
    xml.writeTextElement("top250", QString("%1").arg(episode->top250()));
    xml.writeTextElement("season", episode->seasonNumber().toString());
    xml.writeTextElement("episode", episode->episodeNumber().toString());
    if (episode->displaySeason() != SeasonNumber::NoSeason) {
        xml.writeTextElement("displayseason", episode->displaySeason().toString());
    }
    if (episode->displayEpisode() != EpisodeNumber::NoEpisode) {
        xml.writeTextElement("displayepisode", episode->displayEpisode().toString());
    }
    xml.writeTextElement("plot", episode->overview());
    if (usePlotForOutline() && episode->overview().isEmpty()) {
        xml.writeTextElement("outline", episode->overview());
    }
    xml.writeTextElement("mpaa", episode->certification().toString());
    xml.writeTextElement("playcount", QString("%1").arg(episode->playCount()));
    xml.writeTextElement("lastplayed", episode->lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
    xml.writeTextElement("aired", episode->firstAired().toString("yyyy-MM-dd"));
    xml.writeTextElement("studio", episode->network());
    if (!episode->epBookmark().isNull() && QTime(0, 0, 0).secsTo(episode->epBookmark()) > 0) {
        xml.writeTextElement("epbookmark", QString("%1").arg(QTime(0, 0, 0).secsTo(episode->epBookmark())));
    }

    const auto& writers = episode->writers();
    for (const QString& writer : writers) {
        xml.writeTextElement("credits", writer);
    }

    const auto& directors = episode->directors();
    for (const QString& director : directors) {
        xml.writeTextElement("director", director);
    }
    if (writeThumbUrlsToNfo() && !episode->thumbnail().isEmpty()) {
        xml.writeTextElement("thumb", episode->thumbnail().toString());
    }

    writeActors(xml, episode->actors());

    // officially not supported but scraper providers start to support it
    const auto& tags = episode->tags();
    for (const QString& tag : tags) {
        xml.writeTextElement("tag", tag);
    }

    KodiXml::writeStreamDetails(xml, episode->streamDetails(), {}, episode->streamDetailsLoaded());

    if (!testMode) {
        addMediaelchGeneratorTag(xml);
    }

    xml.writeEndElement();
}

bool EpisodeXmlWriterGeneric::usePlotForOutline() const
{
    return m_usePlotForOutline;
}

void EpisodeXmlWriterGeneric::setUsePlotForOutline(bool usePlotForOutline)
{
    m_usePlotForOutline = usePlotForOutline;
}

} // namespace kodi
} // namespace mediaelch
