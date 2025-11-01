#include "scrapers/movie/imdb/ImdbMovieScrapeJob.h"

#include "globals/Helper.h"
#include "log/Log.h"
#include "network/NetworkRequest.h"
#include "scrapers/imdb/ImdbApi.h"
#include "scrapers/imdb/ImdbJsonParser.h"
#include "scrapers/imdb/ImdbReferencePage.h"
#include "scrapers/movie/imdb/ImdbMovie.h"

#include <QRegularExpression>

#include "scrapers/ScraperUtils.h"
#include "utils/Containers.h"


namespace mediaelch {
namespace scraper {

ImdbMovieScrapeJob::ImdbMovieScrapeJob(ImdbApi& api,
    MovieScrapeJob::Config _config,
    bool loadAllTags,
    QObject* parent) :
    MovieScrapeJob(std::move(_config), parent),
    m_api{api},
    m_imdbId{config().identifier.str()},
    m_loadAllTags{loadAllTags}
{
}

void ImdbMovieScrapeJob::doStart()
{
    m_movie->clear(config().details);
    m_movie->setImdbId(m_imdbId);

    m_api.loadTitle(config().locale, m_imdbId, ImdbApi::PageKind::Reference, [this](QString html, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
            emitFinished();
            return;
        }

        parseAndAssignInfos(html);

        // How many pages do we have to download? Count them.
        m_itemsLeftToDownloads = 1;

        // IMDb has an extra page listing all tags (popular movies can have more than 100 tags).
        if (m_loadAllTags) {
            ++m_itemsLeftToDownloads;
            loadTags();
        }

        // It's possible that none of the above items should be loaded.
        decreaseDownloadCount();
    });
}

void ImdbMovieScrapeJob::loadTags()
{
    const auto cb = [this](QString html, ScraperError error) {
        if (!error.hasError()) {
            parseAndAssignTags(html);

        } else {
            setScraperError(error);
        }
        decreaseDownloadCount();
    };
    m_api.loadTitle(config().locale, m_imdbId, ImdbApi::PageKind::Keywords, cb);
}


void ImdbMovieScrapeJob::parseAndAssignInfos(const QString& html)
{
    ImdbData data = ImdbJsonParser::parseFromReferencePage(html, config().locale);

    if (data.imdbId.isValid()) {
        m_movie->setImdbId(data.imdbId);
    }
    if (data.title.hasValue()) {
        m_movie->setTitle(data.title.value);
    }
    if (data.originalTitle.hasValue()) {
        m_movie->setOriginalTitle(data.originalTitle.value);
    }
    if (data.overview.hasValue()) {
        m_movie->setOverview(data.overview.value);
    }
    if (data.outline.hasValue()) {
        m_movie->setOutline(data.outline.value);
    }
    if (data.tagline.hasValue()) {
        m_movie->setTagline(data.tagline.value);
    }
    if (data.runtime.hasValue()) {
        m_movie->setRuntime(data.runtime.value);
    }
    if (data.released.hasValue()) {
        m_movie->setReleased(data.released.value);
    }
    for (Rating rating : data.ratings) {
        m_movie->ratings().addRating(rating);
    }
    if (data.top250.hasValue()) {
        m_movie->setTop250(data.top250.value);
    }
    if (data.certification.hasValue()) {
        m_movie->setCertification(data.certification.value);
    }
    if (data.poster.hasValue()) {
        m_movie->images().addPoster(data.poster.value);
    }
    if (data.trailer.hasValue()) {
        m_movie->setTrailer(data.trailer.value);
    }
    for (Actor actor : data.actors) {
        m_movie->addActor(actor);
    }
    if (!data.directors.isEmpty()) {
        m_movie->setDirector(setToVector(data.directors).join(", "));
    }
    if (!data.writers.isEmpty()) {
        m_movie->setWriter(setToVector(data.writers).join(", "));
    }
    for (QString genre : data.genres) {
        m_movie->addGenre(genre);
    }
    for (QString studio : data.studios) {
        m_movie->addStudio(studio);
    }
    for (QString country : data.countries) {
        m_movie->addCountry(country);
    }
    for (QString keyword : data.keywords) {
        m_movie->addTag(keyword);
    }
}

void ImdbMovieScrapeJob::parseAndAssignTags(const QString& html)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    if (m_loadAllTags) {
        rx.setPattern(R"(<a[^>]+href="/search/(?:title/\?)keyword[^"]+"\n?>([^<]+)</a>)");
    } else {
        rx.setPattern(R"(<a[^>]+href="/keyword/[^"]+"[^>]*>([^<]+)</a>)");
    }

    QRegularExpressionMatchIterator match = rx.globalMatch(html);
    while (match.hasNext()) {
        m_movie->addTag(match.next().captured(1).trimmed());
    }
}

QString ImdbMovieScrapeJob::sanitizeAmazonMediaUrl(QString url)
{
    // The URL can look like this:
    //   https://m.media-amazon.com/images/M/<image ID>._V1_UY1400_CR90,0,630,1200_AL_.jpg
    // To get the original image, everything after `._V` can be removed.

    if (!url.endsWith(".jpg")) {
        return url;
    }
    QRegularExpression rx(R"re(._V([^/]+).jpg$)re", QRegularExpression::InvertedGreedinessOption);
    url.replace(rx, ".jpg");

    return url;
}

void ImdbMovieScrapeJob::decreaseDownloadCount()
{
    --m_itemsLeftToDownloads;
    if (m_itemsLeftToDownloads == 0) {
        emitFinished();
    }
}

} // namespace scraper
} // namespace mediaelch
