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
    m_movie->clear(config().details);
    m_movie->setImdbId(m_imdbId);

    m_api.loadTitle(config().locale, m_imdbId, ImdbApi::PageKind::Reference, [this](QString html, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
            emitFinished();
            return;
        }

        parseAndAssignInfos(html);
        parseAndAssignPoster(html);
        parseAndStoreActors(html);

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
    using namespace std::chrono;

    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    const QString title = ImdbReferencePage::extractTitle(html);
    if (!title.isEmpty()) {
        m_movie->setTitle(title);
    }
    const QString originalTitle = ImdbReferencePage::extractOriginalTitle(html);
    if (!originalTitle.isEmpty()) {
        m_movie->setOriginalTitle(originalTitle);
    }

    ImdbReferencePage::extractDirectors(m_movie, html);
    ImdbReferencePage::extractWriters(m_movie, html);
    ImdbReferencePage::extractGenres(m_movie, html);
    ImdbReferencePage::extractTaglines(m_movie, html);

    if (!m_loadAllTags) {
        ImdbReferencePage::extractTags(m_movie, html);
    }

    QDate date = ImdbReferencePage::extractReleaseDate(html);
    if (date.isValid()) {
        m_movie->setReleased(date);
    }

    ImdbReferencePage::extractCertification(m_movie, html);

    rx.setPattern(R"re(Runtime</td>.*<li class="ipl-inline-list__item">\n\s+(\d+) min)re");
    match = rx.match(html);

    if (match.hasMatch()) {
        minutes runtime = minutes(match.captured(1).toInt());
        m_movie->setRuntime(runtime);
    }

    rx.setPattern(R"(<h4 class="inline">Runtime:</h4>[^<]*<time datetime="PT([0-9]+)M">)");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->setRuntime(minutes(match.captured(1).toInt()));
    }

    ImdbReferencePage::extractOverview(m_movie, html);
    ImdbReferencePage::extractRating(m_movie, html);
    ImdbReferencePage::extractStudios(m_movie, html);
    ImdbReferencePage::extractCountries(m_movie, html);
}

void ImdbMovieScrapeJob::parseAndStoreActors(const QString& html)
{
    QRegularExpression rx(R"(<table class="cast_list">(.*)</table>)",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match = rx.match(html);
    if (!match.hasMatch()) {
        return;
    }

    const QString content = match.captured(1);
    rx.setPattern(R"(<tr class="[^"]*">(.*)</tr>)");

    QRegularExpressionMatchIterator actorRowsMatch = rx.globalMatch(content);

    while (actorRowsMatch.hasNext()) {
        QString actorHtml = actorRowsMatch.next().captured(1);

        QPair<Actor, QUrl> actorUrl;

        // Name
        rx.setPattern(R"re(<span class="itemprop" itemprop="name">([^<]+)</span>)re");
        match = rx.match(actorHtml);
        if (match.hasMatch()) {
            actorUrl.first.name = match.captured(1).trimmed();
        }

        // URL
        rx.setPattern(R"re(<a href="(/name/[^"]+)")re");
        match = rx.match(actorHtml);
        if (match.hasMatch()) {
            actorUrl.second = QUrl("https://www.imdb.com" + match.captured(1));
        }

        // Character
        rx.setPattern(R"(<td class="character">(.*)</td>)");
        match = rx.match(actorHtml);
        if (match.hasMatch()) {
            QString role = match.captured(1);
            // Everything between <div> and </div>
            rx.setPattern(R"(>(.*)</)");
            match = rx.match(role);
            if (match.hasMatch()) {
                role = match.captured(1);
            }
            actorUrl.first.role = role.remove("(voice)")
                                      .trimmed() //
                                      .replace(QRegularExpression("\\s\\s+"), " ")
                                      .trimmed();
        }

        rx.setPattern(R"re(loadlate="([^"]+)")re");
        match = rx.match(actorHtml);
        if (match.hasMatch()) {
            actorUrl.first.thumb = sanitizeAmazonMediaUrl(match.captured(1));
        }

        m_movie->addActor(actorUrl.first);
        // URL may be empty
        if (actorUrl.second.isValid()) {
            m_actorUrls.push_back(actorUrl);
        }
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

void ImdbMovieScrapeJob::parseAndAssignPoster(const QString& html)
{
    QString regex = QStringLiteral(R"url(<meta property='og:image' content="([^"]+)")url");
    QRegularExpression rx(regex, QRegularExpression::InvertedGreedinessOption);

    QRegularExpressionMatch match = rx.match(html);
    if (match.hasMatch()) {
        const QUrl url(sanitizeAmazonMediaUrl(match.captured(1)));
        if (!url.isValid()) {
            return;
        }

        Poster p;
        p.thumbUrl = url;
        p.originalUrl = url;
        m_movie->images().addPoster(p);
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
