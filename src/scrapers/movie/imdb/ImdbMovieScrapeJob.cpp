#include "scrapers/movie/imdb/ImdbMovieScrapeJob.h"

#include "globals/Helper.h"
#include "log/Log.h"
#include "network/NetworkRequest.h"
#include "scrapers/imdb/ImdbApi.h"
#include "scrapers/imdb/ImdbReferencePage.h"
#include "scrapers/movie/imdb/ImdbMovie.h"

#include <QRegularExpression>

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
    // TODO: no-op
}

void ImdbMovieScrapeJob::loadTags()
{
    // TODO: no-op
}

void ImdbMovieScrapeJob::parseAndAssignInfos(const QString& html, Movie* movie, QSet<MovieScraperInfo> infos) const
{
    using namespace std::chrono;

    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    if (infos.contains(MovieScraperInfo::Title)) {
        const QString title = ImdbReferencePage::extractTitle(html);
        if (!title.isEmpty()) {
            movie->setName(title);
        }
        const QString originalTitle = ImdbReferencePage::extractOriginalTitle(html);
        if (!originalTitle.isEmpty()) {
            movie->setOriginalName(originalTitle);
        }
    }

    if (infos.contains(MovieScraperInfo::Director)) {
        ImdbReferencePage::extractDirectors(movie, html);
    }

    if (infos.contains(MovieScraperInfo::Writer)) {
        ImdbReferencePage::extractWriters(movie, html);
    }

    if (infos.contains(MovieScraperInfo::Genres)) {
        ImdbReferencePage::extractGenres(movie, html);
    }

    if (infos.contains(MovieScraperInfo::Tagline)) {
        ImdbReferencePage::extractTaglines(movie, html);
    }

    if (!m_loadAllTags && infos.contains(MovieScraperInfo::Tags)) {
        ImdbReferencePage::extractTags(movie, html);
    }

    if (infos.contains(MovieScraperInfo::Released)) {
        QDate date = ImdbReferencePage::extractReleaseDate(html);
        if (date.isValid()) {
            movie->setReleased(date);
        }
    }

    if (infos.contains(MovieScraperInfo::Certification)) {
        ImdbReferencePage::extractCertification(movie, html);
    }

    if (infos.contains(MovieScraperInfo::Runtime)) {
        rx.setPattern(R"re(Runtime</td>.*<li class="ipl-inline-list__item">\n\s+(\d+) min)re");
        match = rx.match(html);

        if (match.hasMatch()) {
            minutes runtime = minutes(match.captured(1).toInt());
            movie->setRuntime(runtime);
        }
    }

    rx.setPattern(R"(<h4 class="inline">Runtime:</h4>[^<]*<time datetime="PT([0-9]+)M">)");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Runtime) && match.hasMatch()) {
        movie->setRuntime(minutes(match.captured(1).toInt()));
    }

    if (infos.contains(MovieScraperInfo::Overview)) {
        ImdbReferencePage::extractOverview(movie, html);
    }

    if (infos.contains(MovieScraperInfo::Rating)) {
        ImdbReferencePage::extractRating(movie, html);
    }

    if (infos.contains(MovieScraperInfo::Studios)) {
        ImdbReferencePage::extractStudios(movie, html);
    }

    if (infos.contains(MovieScraperInfo::Countries)) {
        ImdbReferencePage::extractCountries(movie, html);
    }
}

void ImdbMovieScrapeJob::parseAndStoreActors(const QString& html)
{
    // TODO: implement
    Q_UNUSED(html)
}

void ImdbMovieScrapeJob::parseAndAssignTags(const QString& html)
{
    // TODO: implement
    Q_UNUSED(html)
}

void ImdbMovieScrapeJob::parseAndAssignPoster(const QString& html)
{
    // TODO: implement
    Q_UNUSED(html)
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
    // TODO: no-op
}

} // namespace scraper
} // namespace mediaelch
