#include "media_center/kodi/EpisodeXmlWriter.h"

#include "data/tv_show/TvShowEpisode.h"
#include "globals/Helper.h"
#include "media_center/KodiXml.h"
#include "media_center/kodi/EpisodeXmlReader.h"
#include "media_center/kodi/KodiXmlWriter.h"

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

    // unique id: IMDb and TMDB
    // TODO: one uniqueid is required; we don't check that at the moment
    // Empty default unique ID is not valid in Kodi.
    bool hasDefault = false;
    if (episode->tmdbId().isValid()) {
        xml.writeStartElement("uniqueid");
        xml.writeAttribute("type", "tmdb");
        if (!hasDefault) {
            xml.writeAttribute("default", "true");
            hasDefault = true;
        }
        xml.writeCharacters(episode->tmdbId().toString());
        xml.writeEndElement();
    }
    if (episode->imdbId().isValid()) {
        xml.writeStartElement("uniqueid");
        xml.writeAttribute("type", "imdb");
        if (!hasDefault) {
            xml.writeAttribute("default", "true");
            hasDefault = true;
        }
        xml.writeCharacters(episode->imdbId().toString());
        xml.writeEndElement();
    }
    if (episode->tvmazeId().isValid()) {
        xml.writeStartElement("uniqueid");
        xml.writeAttribute("type", "tvmaze");
        if (!hasDefault) {
            xml.writeAttribute("default", "true");
            hasDefault = true;
        }
        xml.writeCharacters(episode->tvmazeId().toString());
        xml.writeEndElement();
    }
    if (episode->tvdbId().isValid()) {
        xml.writeStartElement("uniqueid");
        xml.writeAttribute("type", "tvdb");
        if (!hasDefault) {
            xml.writeAttribute("default", "true");
            hasDefault = true;
        }
        xml.writeCharacters(episode->tvdbId().toString());
        xml.writeEndElement();
    }

    Q_UNUSED(hasDefault)

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
    KodiXml::writeStringsAsOneTagEach(xml, "studio", episode->networks());
    if (!episode->epBookmark().isNull() && QTime(0, 0, 0).secsTo(episode->epBookmark()) > 0) {
        xml.writeTextElement("epbookmark", QString("%1").arg(QTime(0, 0, 0).secsTo(episode->epBookmark())));
    }

    KodiXml::writeStringsAsOneTagEach(xml, "credits", episode->writers());
    KodiXml::writeStringsAsOneTagEach(xml, "director", episode->directors());

    if (writeThumbUrlsToNfo() && !episode->thumbnail().isEmpty()) {
        xml.writeTextElement("thumb", episode->thumbnail().toString());
    }

    writeActors(xml, episode->actors());

    KodiXml::writeStringsAsOneTagEach(xml, "tag", episode->tags());

    KodiXml::writeStreamDetails(xml, episode->streamDetails(), {});

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
