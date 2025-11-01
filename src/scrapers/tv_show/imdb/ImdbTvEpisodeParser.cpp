#include "scrapers/tv_show/imdb/ImdbTvEpisodeParser.h"

#include "data/Poster.h"
#include "data/TvDbId.h"
#include "data/tv_show/TvShowEpisode.h"
#include "globals/Helper.h"
#include "scrapers/ScraperUtils.h"
#include "scrapers/imdb/ImdbReferencePage.h"

#include "scrapers/imdb/ImdbJsonParser.h"

#include <QRegularExpression>
#include <chrono>

#include "utils/Containers.h"

namespace mediaelch {
namespace scraper {

void ImdbTvEpisodeParser::parseInfos(TvShowEpisode& episode, const QString& html, const Locale& preferredLocale)
{
    // Note: Expects HTML from https://www.imdb.com/title/tt________/reference
    using namespace std::chrono;

    ImdbData data = ImdbJsonParser::parseFromReferencePage(html, preferredLocale);

    if (data.imdbId.isValid()) {
        episode.setImdbId(data.imdbId);
    }
    if (data.title.hasValue()) {
        episode.setTitle(data.title.value);
    }
    // Enable once original titles exist for episodes.
    // if (data.originalTitle.hasValue()) {
    //     episode.setOriginalTitle(data.originalTitle.value);
    // }

    if (data.outline.hasValue()) {
        // TODO: We use the outline for the overview; at the moment, we don't distinguish them in TV episodes.
        episode.setOverview(data.outline.value);
    } else if (data.overview.hasValue()) {
        episode.setOverview(data.overview.value);
    }

    if (data.released.hasValue()) {
        episode.setFirstAired(data.released.value);
    }
    for (Rating rating : data.ratings) {
        episode.ratings().addRating(rating);
    }
    if (data.top250.hasValue()) {
        episode.setTop250(data.top250.value);
    }
    if (data.certification.hasValue()) {
        episode.setCertification(data.certification.value);
    }
    for (const Actor& actor : data.actors) {
        episode.addActor(actor);
    }
    if (!data.directors.isEmpty()) {
        episode.setDirectors(setToVector(data.directors));
    }
    if (!data.writers.isEmpty()) {
        episode.setWriters(setToVector(data.writers));
    }
    for (const QString& keyword : data.keywords) {
        episode.addTag(keyword);
    }
    if (data.poster.hasValue()) {
        episode.setThumbnail(data.poster.value.originalUrl);
    }
    // TODO
    // - genres
    // - setNetwork
    // TODO if supported by episode class
    // - runtime
    // - keywords
    // - tagline
}

void ImdbTvEpisodeParser::parseIdFromSeason(TvShowEpisode& episode, const QString& html)
{
    // Example JSON:
    //   ```json
    //   {"id":"tt0696611","type":"tvEpisode","season":"2","episode":"0"…}
    //   ```
    QRegularExpression regex(QStringLiteral(R"re("id":"(tt\d+)","type":"tvEpisode","season":"\d+","episode":"%1")re")
                                 .arg(episode.episodeNumber().toString()),
        QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = regex.match(html);
    if (!match.hasMatch()) {
        return;
    }

    ImdbId imdbId(match.captured(1).trimmed());
    if (imdbId.isValid()) {
        episode.setImdbId(imdbId);
    }
}

} // namespace scraper
} // namespace mediaelch
