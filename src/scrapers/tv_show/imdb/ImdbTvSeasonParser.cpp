#include "scrapers/tv_show/imdb/ImdbTvSeasonParser.h"

#include "data/tv_show/TvShowEpisode.h"
#include "globals/Helper.h"
#include "scrapers/imdb/ImdbJsonParser.h"
#include "scrapers/tv_show/imdb/ImdbTvEpisodeParser.h"

#include <QJsonDocument>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

namespace mediaelch {
namespace scraper {

QSet<SeasonNumber> ImdbTvSeasonParser::parseSeasonNumbersFromEpisodesPage(const QString& html)
{
    QVector<int> seasonList = ImdbJsonParser::parseSeasonNumbersFromEpisodesPage(html);

    QSet<SeasonNumber> seasons;
    for (int season : seasonList) {
        seasons << SeasonNumber(season);
    }

    return seasons;
}

QMap<EpisodeNumber, ImdbId> ImdbTvSeasonParser::parseEpisodeIds(const QString& html, int forSeason)
{
    QVector<ImdbShortEpisodeData> episodesList = ImdbJsonParser::parseEpisodeIds(html);

    QMap<EpisodeNumber, ImdbId> ids;
    for (const ImdbShortEpisodeData& entry : episodesList) {
        if (entry.seasonNumber == forSeason) {
            EpisodeNumber episode(entry.episodeNumber);
            ids.insert(episode, ImdbId(entry.imdbId));
        }
    }

    return ids;
}

} // namespace scraper
} // namespace mediaelch
