#include "scrapers/tv_show/imdb/ImdbTvShowParser.h"

#include "data/tv_show/TvShow.h"
#include "log/Log.h"
#include "scrapers/ScraperInterface.h"
#include "scrapers/ScraperUtils.h"

#include <QDate>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValueRef>
#include <QRegularExpression>
#include <QTextDocument>

#include "scrapers/imdb/ImdbJsonParser.h"
#include "utils/Containers.h"

using namespace std::chrono_literals;

namespace mediaelch {
namespace scraper {

ScraperError ImdbTvShowParser::parseInfos(const QString& html)
{
    ImdbData data = ImdbJsonParser::parseFromReferencePage(html, m_preferredLocale);

    if (data.imdbId.isValid()) {
        m_show.setImdbId(data.imdbId);
    }
    if (data.title.hasValue()) {
        m_show.setTitle(data.title.value);
    }
    if (data.originalTitle.hasValue()) {
        m_show.setOriginalTitle(data.originalTitle.value);
    }

    if (data.outline.hasValue()) {
        // TODO: We use the outline for the overview; at the moment, we don't distinguish them in TV shows.
        m_show.setOverview(data.outline.value);
    } else if (data.overview.hasValue()) {
        m_show.setOverview(data.overview.value);
    }

    if (data.released.hasValue()) {
        m_show.setFirstAired(data.released.value);
    }
    for (Rating rating : data.ratings) {
        m_show.ratings().addRating(rating);
    }
    if (data.top250.hasValue()) {
        m_show.setTop250(data.top250.value);
    }
    if (data.certification.hasValue()) {
        m_show.setCertification(data.certification.value);
    }
    for (const Actor& actor : data.actors) {
        m_show.addActor(actor);
    }
    for (const QString& keyword : data.keywords) {
        m_show.addTag(keyword);
    }
    if (data.poster.hasValue()) {
        m_show.addPoster(data.poster.value);
    }
    for (const QString& genre : data.genres) {
        m_show.addGenre(genre);
    }
    if (data.runtime.hasValue()) {
        m_show.setRuntime(data.runtime.value);
    }

    return ScraperError{};
}

} // namespace scraper
} // namespace mediaelch
