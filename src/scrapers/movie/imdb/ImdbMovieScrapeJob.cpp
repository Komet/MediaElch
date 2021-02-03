#include "scrapers/movie/imdb/ImdbMovieScrapeJob.h"

#include "globals/Helper.h"
#include "movies/Movie.h"
#include "scrapers/imdb/ImdbApi.h"

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

void ImdbMovieScrapeJob::execute()
{
    m_movie->clear(config().details);
    m_movie->setImdbId(m_imdbId);

    m_api.loadMovie(Locale("en"), m_imdbId, [this](QString html, ScraperError error) {
        if (error.hasError()) {
            m_error = error;
            emit sigFinished(this);
            return;
        }

        QUrl posterViewerUrl = parsePosterViewerUrl(html);
        parseAndAssignInfos(html);
        parseAndStoreActors(html);

        const bool shouldLoadPoster = config().details.contains(MovieScraperInfo::Poster) && posterViewerUrl.isValid();
        const bool shouldLoadTags = config().details.contains(MovieScraperInfo::Tags) && m_loadAllTags;
        const bool shouldLoadActors = config().details.contains(MovieScraperInfo::Actors) && !m_actorUrls.isEmpty();

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


void ImdbMovieScrapeJob::loadPoster(const QUrl& posterViewerUrl)
{
    qDebug() << "[ImdbMovieScrapeJob] Loading movie poster detail view";
    m_api.sendGetRequest(Locale("en"), posterViewerUrl, [this](QString html, ScraperError error) {
        if (!error.hasError()) {
            parseAndAssignPoster(html);

        } else {
            m_error = error;
        }
        decreaseDownloadCount();
    });
}

void ImdbMovieScrapeJob::loadTags()
{
    QUrl tagsUrl(QStringLiteral("https://www.imdb.com/title/%1/keywords").arg(m_movie->imdbId().toString()));
    m_api.sendGetRequest(Locale("en"), tagsUrl, [this](QString html, ScraperError error) {
        if (!error.hasError()) {
            parseAndAssignTags(html);

        } else {
            m_error = error;
        }
        decreaseDownloadCount();
    });
}

void ImdbMovieScrapeJob::loadActorImageUrls()
{
    for (int index = 0; index < m_actorUrls.size(); ++index) {
        m_api.sendGetRequest(
            Locale("en"), m_actorUrls[index].second, [actorIndex = index, this](QString html, ScraperError error) {
                if (error.hasError()) {
                    m_error = error;

                } else {
                    QString url = parseActorImageUrl(html);
                    if (!url.isEmpty()) {
                        m_actorUrls[actorIndex].first.thumb = url;
                    }
                }
                decreaseDownloadCount();
            });
    }
}

void ImdbMovieScrapeJob::parseAndStoreActors(const QString& html)
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

        m_movie->addActor(actorUrl.first);
        m_actorUrls.push_back(actorUrl);
    }
}

QUrl ImdbMovieScrapeJob::parsePosterViewerUrl(const QString& html)
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

void ImdbMovieScrapeJob::parseAndAssignTags(const QString& html)
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
        m_movie->addTag(match.next().captured(1).trimmed());
    }
}

QString ImdbMovieScrapeJob::parseActorImageUrl(const QString& html)
{
    QRegularExpression rx(R"re(<link rel=['"]image_src['"] href="([^"]+)">)re", //
        QRegularExpression::InvertedGreedinessOption);

    QRegularExpressionMatch match = rx.match(html);
    if (!match.hasMatch()) {
        return "";
    }

    return match.captured(1);
}

void ImdbMovieScrapeJob::parseAndAssignPoster(const QString& html)
{
    // There should only be one image like this.
    QString regex = QStringLiteral(R"url(<img src="(https://m\.media-amazon\.com/[^"]+)" srcSet=")url");
    QRegularExpression rx(regex, QRegularExpression::InvertedGreedinessOption);

    QRegularExpressionMatch match = rx.match(html);
    if (match.hasMatch()) {
        Poster p;
        p.thumbUrl = match.captured(1);
        p.originalUrl = match.captured(1);
        m_movie->images().addPoster(p);
    }
}

void ImdbMovieScrapeJob::decreaseDownloadCount()
{
    --m_itemsLeftToDownloads;
    if (m_itemsLeftToDownloads == 0) {
        emit sigFinished(this);
    }
}

void ImdbMovieScrapeJob::parseAndAssignInfos(const QString& html)
{
    using namespace std::chrono;

    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    {
        rx.setPattern(R"(<h1 class="[^"]*">([^<]*)&nbsp;)");
        match = rx.match(html);
        if (match.hasMatch()) {
            m_movie->setName(match.captured(1));
        }
        rx.setPattern(R"(<h1 itemprop="name" class="">(.*)&nbsp;<span id="titleYear">)");
        match = rx.match(html);
        if (match.hasMatch()) {
            m_movie->setName(match.captured(1));
        }
        rx.setPattern(R"(<div class="originalTitle">([^<]*)<span)");
        match = rx.match(html);
        if (match.hasMatch()) {
            m_movie->setOriginalName(match.captured(1));
        }
    }

    {
        rx.setPattern(
            R"(<div class="txt-block" itemprop="director" itemscope itemtype="http://schema.org/Person">(.*)</div>)");
        match = rx.match(html);
        QString directorsBlock;
        if (match.hasMatch()) {
            directorsBlock = match.captured(1);
        } else {
            // the ghost span may only exist if there are more than 2 directors
            rx.setPattern(
                R"(<div class="credit_summary_item">\n +<h4 class="inline">Directors?:</h4>(.*)(?:<span class="ghost">|</div>))");
            match = rx.match(html);
            if (match.hasMatch()) {
                directorsBlock = match.captured(1);
            }
        }

        if (!directorsBlock.isEmpty()) {
            QStringList directors;
            rx.setPattern(R"(<a href="[^"]*"[^>]*>([^<]*)</a>)");
            QRegularExpressionMatchIterator directorMatches = rx.globalMatch(directorsBlock);
            while (directorMatches.hasNext()) {
                directors << directorMatches.next().captured(1);
            }
            m_movie->setDirector(directors.join(", "));
        }
    }

    {
        rx.setPattern(
            R"(<div class="txt-block" itemprop="creator" itemscope itemtype="http://schema.org/Person">(.*)</div>)");
        match = rx.match(html);
        QString writersBlock;
        if (match.hasMatch()) {
            writersBlock = match.captured(1);
        } else {
            // the ghost span may only exist if there are more than 2 writers
            rx.setPattern(
                R"(<div class="credit_summary_item">\n +<h4 class="inline">Writers?:</h4>(.*)(?:<span class="ghost">|</div>))");
            match = rx.match(html);
            if (match.hasMatch()) {
                writersBlock = match.captured(1);
            }
        }

        if (!writersBlock.isEmpty()) {
            QStringList writers;
            rx.setPattern(R"(<a href="[^"]*"[^>]*>([^<]*)</a>)");
            QRegularExpressionMatchIterator writerMatches = rx.globalMatch(writersBlock);
            while (writerMatches.hasNext()) {
                writers << writerMatches.next().captured(1);
            }
            m_movie->setWriter(writers.join(", "));
        }
    }

    rx.setPattern(R"(<div class="see-more inline canwrap">\n *<h4 class="inline">Genres:</h4>(.*)</div>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        QString genres = match.captured(1);
        rx.setPattern(R"(<a href="[^"]*"[^>]*>([^<]*)</a>)");
        QRegularExpressionMatchIterator genreMatches = rx.globalMatch(genres);
        while (genreMatches.hasNext()) {
            m_movie->addGenre(helper::mapGenre(genreMatches.next().captured(1).trimmed()));
        }
    }

    rx.setPattern(R"(<div class="txt-block">[^<]*<h4 class="inline">Taglines:</h4>(.*)</div>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        rx.setPattern("<span class=\"see-more inline\">.*</span>");
        const QString tagline = match.captured(1).remove(rx).trimmed();
        m_movie->setTagline(tagline);
    }

    rx.setPattern(R"(<div class="see-more inline canwrap">\n *<h4 class="inline">Plot Keywords:</h4>(.*)<nobr>)");
    match = rx.match(html);
    if (!m_loadAllTags && match.hasMatch()) {
        QString tags = match.captured(1);
        rx.setPattern(R"(<span class="itemprop">([^<]*)</span>)");
        QRegularExpressionMatchIterator tagMatches = rx.globalMatch(tags);
        while (tagMatches.hasNext()) {
            m_movie->addTag(tagMatches.next().captured(1).trimmed());
        }
    }

    {
        rx.setPattern("<a href=\"[^\"]*\"(.*)title=\"See all release dates\" >[^<]*<meta itemprop=\"datePublished\" "
                      "content=\"([^\"]*)\" />");
        match = rx.match(html);
        if (match.hasMatch()) {
            m_movie->setReleased(QDate::fromString(match.captured(2), "yyyy-MM-dd"));

        } else {
            rx.setPattern(R"(<h4 class="inline">Release Date:</h4> ([0-9]+) ([A-z]*) ([0-9]{4}))");
            match = rx.match(html);
            if (match.hasMatch()) {
                int day = match.captured(1).trimmed().toInt();
                int month = -1;
                QString monthName = match.captured(2).trimmed();
                int year = match.captured(3).trimmed().toInt();
                if (monthName.contains("January", Qt::CaseInsensitive)) {
                    month = 1;
                } else if (monthName.contains("February", Qt::CaseInsensitive)) {
                    month = 2;
                } else if (monthName.contains("March", Qt::CaseInsensitive)) {
                    month = 3;
                } else if (monthName.contains("April", Qt::CaseInsensitive)) {
                    month = 4;
                } else if (monthName.contains("May", Qt::CaseInsensitive)) {
                    month = 5;
                } else if (monthName.contains("June", Qt::CaseInsensitive)) {
                    month = 6;
                } else if (monthName.contains("July", Qt::CaseInsensitive)) {
                    month = 7;
                } else if (monthName.contains("August", Qt::CaseInsensitive)) {
                    month = 8;
                } else if (monthName.contains("September", Qt::CaseInsensitive)) {
                    month = 9;
                } else if (monthName.contains("October", Qt::CaseInsensitive)) {
                    month = 10;
                } else if (monthName.contains("November", Qt::CaseInsensitive)) {
                    month = 11;
                } else if (monthName.contains("December", Qt::CaseInsensitive)) {
                    month = 12;
                }

                if (day != 0 && month != -1 && year != 0) {
                    m_movie->setReleased(QDate(year, month, day));
                }

            } else {
                rx.setPattern(R"(<title>[^<]+(?:\(| )(\d{4})\) - IMDb</title>)");
                match = rx.match(html);
                if (match.hasMatch()) {
                    const int day = 1;
                    const int month = 1;
                    const int year = match.captured(1).trimmed().toInt();
                    m_movie->setReleased(QDate(year, month, day));
                }
            }
        }
    }

    rx.setPattern(R"rx("contentRating": "([^"]*)",)rx");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->setCertification(helper::mapCertification(Certification(match.captured(1))));
    }

    rx.setPattern(R"("duration": "PT([0-9]+)H?([0-9]+)M")");
    match = rx.match(html);
    if (match.hasMatch()) {
        if (rx.captureCount() > 1) {
            minutes runtime = hours(match.captured(1).toInt()) + minutes(match.captured(2).toInt());
            m_movie->setRuntime(runtime);
        } else {
            minutes runtime = minutes(match.captured(1).toInt());
            m_movie->setRuntime(runtime);
        }
    }

    rx.setPattern(R"(<h4 class="inline">Runtime:</h4>[^<]*<time datetime="PT([0-9]+)M">)");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->setRuntime(minutes(match.captured(1).toInt()));
    }

    rx.setPattern("<p itemprop=\"description\">(.*)</p>");
    match = rx.match(html);
    if (match.hasMatch()) {
        QString outline = match.captured(1).remove(QRegularExpression("<[^>]*>"));
        outline = outline.remove("See full summary&nbsp;&raquo;").trimmed();
        m_movie->setOutline(outline);
    }

    rx.setPattern(R"(<div class="summary_text">(.*)</div>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        QString outline = match.captured(1).remove(QRegularExpression("<[^>]*>"));
        outline = outline.remove("See full summary&nbsp;&raquo;").trimmed();
        m_movie->setOutline(outline);
    }

    rx.setPattern(R"(<h2>Storyline</h2>\n +\n +<div class="inline canwrap">\n +<p>\n +<span>(.*)</span>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        QString overview = match.captured(1).trimmed();
        overview.remove(QRegularExpression("<[^>]*>"));
        m_movie->setOverview(overview.trimmed());
    }

    {
        Rating rating;
        rating.source = "imdb";
        rating.maxRating = 10;
        rx.setPattern("<div class=\"star-box-details\" itemtype=\"http://schema.org/AggregateRating\" itemscope "
                      "itemprop=\"aggregateRating\">(.*)</div>");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString content = match.captured(1);
            rx.setPattern("<span itemprop=\"ratingValue\">(.*)</span>");
            match = rx.match(content);
            if (match.hasMatch()) {
                rating.rating = match.captured(1).trimmed().replace(",", ".").toDouble();
            }

            rx.setPattern("<span itemprop=\"ratingCount\">(.*)</span>");
            match = rx.match(content);
            if (match.hasMatch()) {
                rating.voteCount = match.captured(1).replace(",", "").replace(".", "").toInt();
            }
        } else {
            rx.setPattern(R"(<div class="imdbRating"[^>]*>\n +<div class="ratingValue">(.*)</div>)");
            match = rx.match(html);
            if (match.hasMatch()) {
                QString content = match.captured(1);
                rx.setPattern("([0-9]\\.[0-9]) based on ([0-9\\,]*) ");
                match = rx.match(content);
                if (match.hasMatch()) {
                    rating.rating = match.captured(1).trimmed().replace(",", ".").toDouble();
                    rating.voteCount = match.captured(2).replace(",", "").replace(".", "").toInt();
                }
                rx.setPattern("([0-9]\\,[0-9]) based on ([0-9\\.]*) ");
                match = rx.match(content);
                if (match.hasMatch()) {
                    rating.rating = match.captured(1).trimmed().replace(",", ".").toDouble();
                    rating.voteCount = match.captured(2).replace(",", "").replace(".", "").toInt();
                }
            }
        }

        m_movie->ratings().push_back(rating);

        // Top250 for movies
        rx.setPattern("Top Rated Movies #([0-9]+)\\n</a>");
        match = rx.match(html);
        if (match.hasMatch()) {
            m_movie->setTop250(match.captured(1).toInt());
        }
        // Top250 for TV shows (used by TheTvDb)
        rx.setPattern("Top Rated TV #([0-9]+)\\n</a>");
        match = rx.match(html);
        if (match.hasMatch()) {
            m_movie->setTop250(match.captured(1).toInt());
        }
    }

    rx.setPattern(R"(<h4 class="inline">Production Co:</h4>(.*)<span class="see-more inline">)");
    match = rx.match(html);
    if (match.hasMatch()) {
        QString studios = match.captured(1);
        rx.setPattern(R"(<a href="/company/[^"]*"[^>]*>([^<]+)</a>)");
        QRegularExpressionMatchIterator studioMatches = rx.globalMatch(studios);
        while (studioMatches.hasNext()) {
            m_movie->addStudio(helper::mapStudio(studioMatches.next().captured(1).trimmed()));
        }
    }

    rx.setPattern(R"(<h4 class="inline">Country:</h4>(.*)</div>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        QString content = match.captured(1);
        rx.setPattern(R"(<a href="[^"]*"[^>]*>([^<]*)</a>)");
        QRegularExpressionMatchIterator countryMatches = rx.globalMatch(content);
        while (countryMatches.hasNext()) {
            m_movie->addCountry(helper::mapCountry(countryMatches.next().captured(1).trimmed()));
        }
    }
}

} // namespace scraper
} // namespace mediaelch
