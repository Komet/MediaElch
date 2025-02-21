#include "scrapers/movie/hotmovies/HotMoviesScrapeJob.h"

#include "data/movie/Movie.h"
#include "scrapers/movie/hotmovies/HotMoviesApi.h"

#include <QRegularExpression>
#include <QTextDocument>
#include <QTextDocumentFragment>

namespace mediaelch {
namespace scraper {

HotMoviesScrapeJob::HotMoviesScrapeJob(HotMoviesApi& api, MovieScrapeJob::Config _config, QObject* parent) :
    MovieScrapeJob(std::move(_config), parent), m_api{api}
{
}

void HotMoviesScrapeJob::doStart()
{
    m_api.loadMovie(config().identifier.str(), [this](QString data, ScraperError error) {
        if (!error.hasError()) {
            parseAndAssignInfos(data);

        } else {
            setScraperError(error);
        }

        emitFinished();
    });
}

void HotMoviesScrapeJob::parseAndAssignInfos(const QString& html)
{
    static QRegularExpression numberRx("\\d+"); // must be greedy

    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match;

    rx.setPattern(R"(<h1 class="[^"]+"(?: itemprop="name")?>(.*)</h1>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        QString name = decodeAndTrim(match.captured(1));
        name.remove("Kick Off Sale");
        m_movie->setTitle(name.trimmed());
    }

    // Rating currently not available; HotMovies has switched to likes
    // rx.setPattern("<meta itemprop=\"ratingValue\" content=\"(.*)\">");
    // if (match.hasMatch()) {
    //     m_movie->setRating(match.captured(1).toDouble());
    // }

    // Only the main like count has text after the thumbs-up-count
    // In 2019, it contained a link (therefore `</a>`).
    // As of 2020-04-05 this wasn't the case anymore but as of 2022-06-05 it's there again.
    // As of 2023-11-25, no votes are available
    // rx.setPattern(R"(<span class="thumbs-up-count">(\d+)</span>(</a>)?<br /><span class="thumbs-up-text")");
    // match = rx.match(html);
    // if (match.hasMatch()) {
    //     Rating rating;
    //     rating.voteCount = match.captured(1).toInt();
    //     rating.source = "HotMovies";
    //     m_movie->ratings().setOrAddRating(rating);
    // }

    rx.setPattern(R"(<strong>Release Year:</strong> ?(\d{4}))");
    match = rx.match(html);
    if (match.hasMatch()) {
        QDate date = QDate::fromString(match.captured(1), "yyyy");
        if (date.isValid()) {
            m_movie->setReleased(date);
        }
    }

    rx.setPattern(R"(<strong>Released:</strong> ?([A-Za-z]+ \d{2} \d{4}))");
    match = rx.match(html);
    if (match.hasMatch()) {
        QDate date = QDate::fromString(match.captured(1), "MMM dd yyyy");
        if (date.isValid()) {
            m_movie->setReleased(date);
        }
    }

    rx.setPattern(R"re(Run Time:\s</[^>]+>\s+((?:\d+\s+hrs\.\s+)?\d+\s+min))re");
    match = rx.match(html);
    if (match.hasMatch()) {
        using namespace std::chrono;

        QRegularExpressionMatchIterator matches = numberRx.globalMatch(match.captured(1));

        QString first = matches.hasNext() ? matches.next().captured(0) : "";
        QString second = matches.hasNext() ? matches.next().captured(0) : "";

        if (!second.isEmpty()) {
            minutes runtime = hours(first.toInt()) + minutes(second.toInt());
            m_movie->setRuntime(runtime);

        } else if (!first.isEmpty()) {
            minutes runtime = minutes(first.toInt());
            m_movie->setRuntime(runtime);
        }
    }

    rx.setPattern(R"(<article>.*</h2>(.*)</article>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->setOverview(decodeAndTrim(match.captured(1)));
    }

    rx.setPattern(R"rx(id="front-cover".+>.*<img src="(https://[^"]+)")rx");
    match = rx.match(html);
    if (match.hasMatch()) {
        Poster p;
        p.thumbUrl = match.captured(1);
        p.originalUrl = match.captured(1);
        m_movie->images().addPoster(p);
    }

    rx.setPattern(R"rx(<a href="([^"]+)"[\s\n]+class="[^"]+"[\s\n]+rel="boxcoverAlt")rx");
    match = rx.match(html);
    if (match.hasMatch()) {
        Poster p;
        p.thumbUrl = match.captured(1);
        p.originalUrl = match.captured(1);
        // add both as additional poster and backdrop (fanart)
        // TODO: Add as "posterN" when we support it
        m_movie->images().addPoster(p);
        m_movie->images().addBackdrop(p);
    }

    rx.setPattern(R"re(<strong>Starring:</strong>(.*)</div>)re");
    match = rx.match(html);
    if (match.hasMatch()) {
        // clear actors
        m_movie->setActors({});
        rx.setPattern(R"re(Label="Performer">(.*)</a>)re");
        QRegularExpressionMatchIterator matches = rx.globalMatch(match.captured(1));
        while (matches.hasNext()) {
            match = matches.next();
            Actor a;
            a.name = decodeAndTrim(decodeAndTrim(match.captured(1)));
            const auto pictureUrl = match.captured(1);
            // if (!pictureUrl.endsWith("missing_f.gif") && !pictureUrl.endsWith("missing_m.gif")) {
            //     a.thumb = pictureUrl;
            // }
            m_movie->addActor(a);
        }
    }

    {
        rx.setPattern("Label=\"Category\">([^<]+)</a>");
        QRegularExpressionMatchIterator matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            m_movie->addGenre(decodeAndTrim(matches.next().captured(1)));
        }
    }

    rx.setPattern(R"re(<strong>Studio:</strong> <a(?: itemprop="url")? href="[^"]*"[\s\t\n]*title="([^"]*)")re");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->addStudio(decodeAndTrim(match.captured(1)));
    }

    rx.setPattern(R"re("director":\[\{"@type":"Person","name":"([^"]+)")re");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->setDirector(decodeAndTrim(match.captured(1)));
    }

    rx.setPattern(R"re(<strong>Series:</strong>\s+<a href="/series/[^"]+">(.*)</a>)re");
    match = rx.match(html);
    if (match.hasMatch()) {
        MovieSet set;
        set.name = decodeAndTrim(match.captured(1));
        m_movie->setSet(set);
    }
}

QString HotMoviesScrapeJob::decodeAndTrim(const QString& htmlEncodedString)
{
    return QTextDocumentFragment::fromHtml(htmlEncodedString).toPlainText().trimmed();
}

} // namespace scraper
} // namespace mediaelch
