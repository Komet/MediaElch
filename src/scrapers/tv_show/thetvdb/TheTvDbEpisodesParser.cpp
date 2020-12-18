#include "scrapers/tv_show/thetvdb/TheTvDbEpisodesParser.h"

#include "globals/Helper.h"
#include "scrapers/tv_show/thetvdb/TheTvDbEpisodeParser.h"
#include "tv_shows/TvShowEpisode.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

namespace mediaelch {
namespace scraper {

TheTvDbApi::Paginate TheTvDbEpisodesParser::parseEpisodes(const QJsonDocument& json,
    SeasonOrder seasonOrder,
    QObject* parentForEpisodes,
    std::function<void(TvShowEpisode*)> episodeCallback)
{
    const auto parsedJson = json.object();
    const auto paginateObj = parsedJson.value("links").toObject();
    const auto episodesArray = parsedJson.value("data").toArray();

    for (const auto& episodeValue : episodesArray) {
        const auto episodeObj = episodeValue.toObject();
        auto* episode = new TvShowEpisode({}, parentForEpisodes);
        TheTvDbEpisodeParser parser(*episode, seasonOrder);
        parser.parseInfos(episodeObj);
        if (episode->seasonNumber() == SeasonNumber::NoSeason || episode->episodeNumber() == EpisodeNumber::NoEpisode) {
            episode->deleteLater();
        } else {
            episodeCallback(episode);
        }
    }

    TheTvDbApi::Paginate p;
    p.first = paginateObj.value("first").toInt();
    p.last = paginateObj.value("last").toInt();
    p.next = paginateObj.value("next").toInt();
    p.prev = paginateObj.value("prev").toInt();
    return p;
}

} // namespace scraper
} // namespace mediaelch
