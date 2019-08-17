#include "media_centers/kodi/v17/EpisodeXmlWriterV17.h"

#include "globals/Helper.h"
#include "media_centers/KodiXml.h"
#include "settings/Settings.h"
#include "tv_shows/TvShowEpisode.h"

#include <QDomDocument>

namespace mediaelch {
namespace kodi {

EpisodeXmlWriterV17::EpisodeXmlWriterV17(QVector<TvShowEpisode*> episodes) : m_episodes{std::move(episodes)}
{
}

QByteArray EpisodeXmlWriterV17::getEpisodeXml()
{
    QByteArray xmlContent;
    QXmlStreamWriter xml(&xmlContent);
    xml.setAutoFormatting(true);
    xml.writeStartDocument("1.0", true);

    for (TvShowEpisode* subEpisode : m_episodes) {
        writeSingleEpisodeDetails(xml, subEpisode);
    }

    xml.writeEndDocument();
    return xmlContent;
}


/// @brief Writes tv show episode elements to an xml stream
/// @param xml XML stream
/// @param episode Episode to save
void EpisodeXmlWriterV17::writeSingleEpisodeDetails(QXmlStreamWriter& xml, TvShowEpisode* episode)
{
    xml.writeStartElement("episodedetails");
    xml.writeTextElement("imdbid", episode->imdbId().toString());
    xml.writeTextElement("title", episode->name());
    xml.writeTextElement("showtitle", episode->showTitle());
    xml.writeTextElement("rating", QString("%1").arg(episode->rating()));
    xml.writeTextElement("votes", QString("%1").arg(episode->votes()));
    xml.writeTextElement("top250", QString("%1").arg(episode->top250()));
    xml.writeTextElement("season", episode->season().toString());
    xml.writeTextElement("episode", episode->episode().toString());
    if (episode->displaySeason() != SeasonNumber::NoSeason) {
        xml.writeTextElement("displayseason", episode->displaySeason().toString());
    }
    if (episode->displayEpisode() != EpisodeNumber::NoEpisode) {
        xml.writeTextElement("displayepisode", episode->displayEpisode().toString());
    }
    xml.writeTextElement("plot", episode->overview());
    xml.writeTextElement("outline", episode->overview());
    xml.writeTextElement("mpaa", episode->certification().toString());
    xml.writeTextElement("playcount", QString("%1").arg(episode->playCount()));
    xml.writeTextElement("lastplayed", episode->lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
    xml.writeTextElement("aired", episode->firstAired().toString("yyyy-MM-dd"));
    xml.writeTextElement("studio", episode->network());
    if (!episode->epBookmark().isNull() && QTime(0, 0, 0).secsTo(episode->epBookmark()) > 0) {
        xml.writeTextElement("epbookmark", QString("%1").arg(QTime(0, 0, 0).secsTo(episode->epBookmark())));
    }
    for (const QString& writer : episode->writers()) {
        xml.writeTextElement("credits", writer);
    }

    for (const QString& director : episode->directors()) {
        xml.writeTextElement("director", director);
    }
    if (Settings::instance()->advanced()->writeThumbUrlsToNfo() && !episode->thumbnail().isEmpty()) {
        xml.writeTextElement("thumb", episode->thumbnail().toString());
    }

    for (const Actor& actor : episode->actors()) {
        xml.writeStartElement("actor");
        xml.writeTextElement("name", actor.name);
        xml.writeTextElement("role", actor.role);
        if (!actor.thumb.isEmpty() && Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
            xml.writeTextElement("thumb", actor.thumb);
        }
        xml.writeEndElement();
    }

    KodiXml::writeStreamDetails(xml, episode->streamDetails());

    xml.writeEndElement();
}

} // namespace kodi
} // namespace mediaelch
