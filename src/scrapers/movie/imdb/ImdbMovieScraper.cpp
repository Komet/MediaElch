#include "scrapers/movie/imdb/ImdbMovieScraper.h"

#include "globals/Helper.h"
#include "log/Log.h"
#include "network/NetworkRequest.h"
#include "scrapers/imdb/ImdbApi.h"
#include "scrapers/movie/imdb/ImdbMovie.h"

#include <QRegularExpression>

namespace mediaelch {
namespace scraper {

void ImdbMovieLoader::load()
{
    m_movie.setImdbId(m_imdbId);

    m_api.loadTitle(Locale("en"), m_imdbId, ImdbApi::PageKind::Reference, [this](QString html, ScraperError error) {
        if (error.hasError()) {
            // TODO
            m_scraper.showNetworkError(error);
            emit sigLoadDone(m_movie, this);
            return;
        }

        m_movie.clear(m_infos);

        parseAndAssignInfos(html);
        parseAndAssignPoster(html);
        parseAndStoreActors(html);

        const bool shouldLoadTags = m_infos.contains(MovieScraperInfo::Tags) && m_loadAllTags;

        // How many pages do we have to download? Count them.
        m_itemsLeftToDownloads = 1;

        // IMDb has an extra page listing all tags (popular movies can have more than 100 tags).
        if (shouldLoadTags) {
            ++m_itemsLeftToDownloads;
            loadTags();
        }

        // It's possible that none of the above items should be loaded.
        decreaseDownloadCount();
    });
}

void ImdbMovieLoader::loadTags()
{
    const auto cb = [this](QString html, ScraperError error) {
        if (!error.hasError()) {
            parseAndAssignTags(html);

        } else {
            // TODO
            m_scraper.showNetworkError(error);
        }
        decreaseDownloadCount();
    };
    m_api.loadTitle(Locale("en"), m_imdbId, ImdbApi::PageKind::Keywords, cb);
}

void ImdbMovieLoader::parseAndAssignInfos(const QString& html)
{
    m_scraper.parseAndAssignInfos(html, &m_movie, m_infos);
}

void ImdbMovieLoader::parseAndStoreActors(const QString& html)
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

        m_movie.addActor(actorUrl.first);
        // URL may be empty
        if (actorUrl.second.isValid()) {
            m_actorUrls.push_back(actorUrl);
        }
    }
}

void ImdbMovieLoader::parseAndAssignTags(const QString& html)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    if (m_loadAllTags) {
        rx.setPattern(R"(<a href="/search/keyword[^"]+"\n?>([^<]+)</a>)");
    } else {
        rx.setPattern(R"(<a href="/keyword/[^"]+"[^>]*>([^<]+)</a>)");
    }


    QRegularExpressionMatchIterator match = rx.globalMatch(html);
    while (match.hasNext()) {
        m_movie.addTag(match.next().captured(1).trimmed());
    }
}

void ImdbMovieLoader::mergeActors()
{
    // Simple brute-force merge.
    // \todo This code can most likely be simplified
    for (const auto& actorUrlPair : asConst(m_actorUrls)) {
        for (Actor* actor : m_movie.actors()) {
            if (actor->name == actorUrlPair.first.name) {
                actor->thumb = actorUrlPair.first.thumb;
                break;
            }
        }
    }
}

void ImdbMovieLoader::parseAndAssignPoster(const QString& html)
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
        m_movie.images().addPoster(p);
    }
}

QString ImdbMovieLoader::sanitizeAmazonMediaUrl(QString url)
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

void ImdbMovieLoader::decreaseDownloadCount()
{
    --m_itemsLeftToDownloads;
    if (m_itemsLeftToDownloads == 0) {
        mergeActors();
        emit sigLoadDone(m_movie, this);
    }
}

} // namespace scraper
} // namespace mediaelch
