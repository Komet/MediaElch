#include "scrapers/tv_show/imdb/ImdbTvSeasonParser.h"

#include "globals/Helper.h"
#include "scrapers/tv_show/imdb/ImdbTvEpisodeParser.h"
#include "tv_shows/TvShowEpisode.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>

namespace mediaelch {
namespace scraper {

QSet<SeasonNumber> ImdbTvSeasonParser::parseSeasonNumbersFromEpisodesPage(const QString& html)
{
    QRegularExpression regex(R"re(<select id="bySeason" tconst="tt\d+" class="current">(.*?)</select>)re",
        QRegularExpression::DotMatchesEverythingOption);

    QRegularExpressionMatch selectMatch = regex.match(html);
    if (!selectMatch.hasMatch()) {
        return {};
    }

    const QString selectWithSeasonOptions = selectMatch.captured(1);
    regex.setPattern(R"re(<option\s+(?:selected="selected"\s+)?value="(\d+)">)re");
    QRegularExpressionMatchIterator i = regex.globalMatch(selectWithSeasonOptions);

    QSet<SeasonNumber> seasons;
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        bool ok = false;
        const int season = match.captured(1).toInt(&ok);
        if (ok && season >= 0) {
            seasons << SeasonNumber(season);
        }
    }

    return seasons;
}

QMap<EpisodeNumber, ImdbId> ImdbTvSeasonParser::parseEpisodeIds(const QString& html)
{
    QRegularExpression regex(R"re(<a href="/title/(tt\d+)/\?ref_=ttep_ep(\d+)")re");

    QMap<EpisodeNumber, ImdbId> ids;
    QRegularExpressionMatchIterator i = regex.globalMatch(html);

    QSet<SeasonNumber> seasons;
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        bool ok = false;
        int episodeNumber = match.captured(2).toInt(&ok);
        if (ok && ImdbId::isValidFormat(match.captured(1)) && episodeNumber >= 0) {
            ImdbId id(match.captured(1));
            EpisodeNumber episode(episodeNumber);
            ids.insert(episode, id);
        }
    }

    return ids;
}

} // namespace scraper
} // namespace mediaelch
