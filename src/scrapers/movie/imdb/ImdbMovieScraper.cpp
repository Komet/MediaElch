#include "scrapers/movie/imdb/ImdbMovieScraper.h"

#include "globals/Helper.h"
#include "network/NetworkRequest.h"
#include "scrapers/imdb/ImdbApi.h"
#include "scrapers/movie/imdb/ImdbMovie.h"

#include <QRegularExpression>

namespace mediaelch {
namespace scraper {

void ImdbMovieLoader::load()
{
    m_movie.clear(m_infos);
    m_movie.setImdbId(m_imdbId);

    m_api.loadMovie(Locale("en"), m_imdbId, [this](QString html, ScraperError error) {
        if (error.hasError()) {
            // TODO
            m_scraper.showNetworkError(error);
            emit sigLoadDone(m_movie, this);
            return;
        }

        QUrl posterViewerUrl = parsePosterViewerUrl(html);
        parseAndAssignInfos(html);
        parseAndStoreActors(html);

        const bool shouldLoadPoster = m_infos.contains(MovieScraperInfo::Poster) && posterViewerUrl.isValid();
        const bool shouldLoadTags = m_infos.contains(MovieScraperInfo::Tags) && m_loadAllTags;
        const bool shouldLoadActors = m_infos.contains(MovieScraperInfo::Actors) && !m_actorUrls.isEmpty();

        { // How many pages do we have to download? Count them.
            m_itemsLeftToDownloads = 1;
            if (shouldLoadPoster) {
                ++m_itemsLeftToDownloads;
            }
            // IMDb has an extra page listing all tags (popular movies can have more than 100 tags).
            if (shouldLoadTags) {
                ++m_itemsLeftToDownloads;
            }
            if (shouldLoadActors) {
                m_itemsLeftToDownloads += m_actorUrls.size();
            }
        }

        if (shouldLoadPoster) {
            loadPoster(posterViewerUrl);
        }
        if (shouldLoadTags) {
            loadTags();
        }
        if (shouldLoadActors) {
            loadActorImageUrls();
        }
        // It's possible that none of the above items should be loaded.
        decreaseDownloadCount();
    });
}


void ImdbMovieLoader::loadPoster(const QUrl& posterViewerUrl)
{
    qDebug() << "[ImdbMovieLoader] Loading movie poster detail view";
    m_api.sendGetRequest(Locale("en"), posterViewerUrl, [this](QString html, ScraperError error) {
        if (!error.hasError()) {
            parseAndAssignPoster(html);

        } else {
            // TODO
            m_scraper.showNetworkError(error);
        }
        decreaseDownloadCount();
    });
}

void ImdbMovieLoader::loadTags()
{
    QUrl tagsUrl(QStringLiteral("https://www.imdb.com/title/%1/keywords").arg(m_movie.imdbId().toString()));
    m_api.sendGetRequest(Locale("en"), tagsUrl, [this](QString html, ScraperError error) {
        if (!error.hasError()) {
            parseAndAssignTags(html);

        } else {
            // TODO
            m_scraper.showNetworkError(error);
        }
        decreaseDownloadCount();
    });
}

void ImdbMovieLoader::loadActorImageUrls()
{
    for (int index = 0; index < m_actorUrls.size(); ++index) {
        m_api.sendGetRequest(
            Locale("en"), m_actorUrls[index].second, [actorIndex = index, this](QString html, ScraperError error) {
                if (error.hasError()) {
                    // TODO
                    m_scraper.showNetworkError(error);
                    decreaseDownloadCount();
                    return;
                }

                QString url = parseActorImageUrl(html);
                if (!url.isEmpty()) {
                    m_actorUrls[actorIndex].first.thumb = url;
                }
                decreaseDownloadCount();
            });
    }
}

void ImdbMovieLoader::parseAndAssignInfos(const QString& html)
{
    m_scraper.parseAndAssignInfos(html, &m_movie, m_infos);
}

void ImdbMovieLoader::parseAndStoreActors(const QString& html)
{
    QRegularExpression rx("<table class=\"cast_list\">(.*)</table>",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match = rx.match(html);
    if (!match.hasMatch()) {
        return;
    }

    QString content = match.captured(1);
    rx.setPattern(R"(<tr class="[^"]*">(.*)</tr>)");

    QRegularExpressionMatchIterator actorRowsMatch = rx.globalMatch(content);

    while (actorRowsMatch.hasNext()) {
        QString actorHtml = actorRowsMatch.next().captured(1);

        QPair<Actor, QUrl> actorUrl;

        rx.setPattern(R"re(<a href="(/name/[^"]+)"\n\s*>([^<]*)</a>)re");
        match = rx.match(actorHtml);
        if (match.hasMatch()) {
            actorUrl.second = QUrl("https://www.imdb.com" + match.captured(1));
            actorUrl.first.name = match.captured(2).trimmed();
        }

        rx.setPattern(R"(<td class="character">\n\s*(.*)</td>)");
        match = rx.match(actorHtml);
        if (match.hasMatch()) {
            QString role = match.captured(1);
            rx.setPattern(R"(<a href="[^"]*" >([^<]*)</a>)");
            match = rx.match(role);
            if (match.hasMatch()) {
                role = match.captured(1);
            }
            actorUrl.first.role = role.trimmed().replace(QRegularExpression("[\\s\\n]+"), " ");
        }

        rx.setPattern("<img [^<]*loadlate=\"([^\"]*)\"[^<]* />");
        match = rx.match(actorHtml);
        if (match.hasMatch()) {
            QString img = match.captured(1);
            rx.setPattern("https://ia.media-imdb.com/images/(.*)/(.*)._V(.*).jpg");
            match = rx.match(img);
            if (match.hasMatch()) {
                actorUrl.first.thumb =
                    "https://ia.media-imdb.com/images/" + match.captured(1) + "/" + match.captured(2) + ".jpg";
            } else {
                actorUrl.first.thumb = match.captured(1);
            }
        }

        m_movie.addActor(actorUrl.first);
        m_actorUrls.push_back(actorUrl);
    }
}

QUrl ImdbMovieLoader::parsePosterViewerUrl(const QString& html)
{
    QRegularExpression rx("<div class=\"poster\">(.*)</div>",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match = rx.match(html);
    if (!match.hasMatch()) {
        return QUrl();
    }

    const QString content = match.captured(1);
    rx.setPattern("<a href=\"/title/(tt[^\"]*)\"");
    match = rx.match(content);
    if (!match.hasMatch()) {
        return QUrl();
    }

    return QStringLiteral("https://www.imdb.com/title/%1").arg(match.captured(1));
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

QString ImdbMovieLoader::parseActorImageUrl(const QString& html)
{
    QRegularExpression rx(R"re(<link rel=['"]image_src['"] href="([^"]+)">)re", //
        QRegularExpression::InvertedGreedinessOption);

    QRegularExpressionMatch match = rx.match(html);
    if (!match.hasMatch()) {
        return "";
    }

    return match.captured(1);
}

void ImdbMovieLoader::mergeActors()
{
    // Simple brute-force merge.
    // \todo This code can most likely be simplified
    for (const auto& actorUrlPair : m_actorUrls) {
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
    // There should only be one image like this.
    QString regex = QStringLiteral(R"url(<img src="(https://m\.media-amazon\.com/[^"]+)" srcSet=")url");
    QRegularExpression rx(regex, QRegularExpression::InvertedGreedinessOption);

    QRegularExpressionMatch match = rx.match(html);
    if (match.hasMatch()) {
        Poster p;
        p.thumbUrl = match.captured(1);
        p.originalUrl = match.captured(1);
        m_movie.images().addPoster(p);
    }
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
