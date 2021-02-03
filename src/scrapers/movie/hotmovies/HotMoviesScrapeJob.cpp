#include "scrapers/movie/hotmovies/HotMoviesScrapeJob.h"

#include "movies/Movie.h"
#include "scrapers/movie/hotmovies/HotMoviesApi.h"

#include <QTextDocument>

namespace mediaelch {
namespace scraper {

HotMoviesScrapeJob::HotMoviesScrapeJob(HotMoviesApi& api, MovieScrapeJob::Config _config, QObject* parent) :
    MovieScrapeJob(std::move(_config), parent), m_api{api}
{
}

void HotMoviesScrapeJob::execute()
{
    m_api.loadMovie(config().identifier.str(), [this](QString data, ScraperError error) {
        if (!error.hasError()) {
            parseAndAssignInfos(data);

        } else {
            m_error = error;
        }

        emit sigFinished(this);
    });
}

void HotMoviesScrapeJob::parseAndAssignInfos(const QString& html)
{
    QRegExp rx;
    rx.setMinimal(true);

    rx.setPattern(R"(<h1 class="title"(?: itemprop="name")?>(.*)</h1>)");
    if (rx.indexIn(html) != -1) {
        m_movie->setName(rx.cap(1));
    }

    // Rating currently not available; HotMovies has switched to likes
    // rx.setPattern("<meta itemprop=\"ratingValue\" content=\"(.*)\">");
    // if (infos.contains(MovieScraperInfo::Rating) && rx.indexIn(html) != -1) {
    //     m_movie->setRating(rx.cap(1).toDouble());
    // }

    // Only the main like count has text after the thumbs-up-count
    // In 2019, it contained a link (therefore `</a>`).
    // As of 2020-04-05 this is not the case anymore.
    rx.setPattern(R"(<span class="thumbs-up-count">(\d+)</span>(</a>)?<br /><span class="thumbs-up-text">)");
    if (rx.indexIn(html) != -1) {
        Rating rating;
        rating.voteCount = rx.cap(1).toInt();
        rating.source = "HotMovies";
        m_movie->ratings().push_back(rating);
    }

    rx.setPattern("<strong>Released:</strong> ?([0-9]{4})");
    if (rx.indexIn(html) != -1) {
        m_movie->setReleased(QDate::fromString(rx.cap(1), "yyyy"));
    }

    rx.setPattern(R"(<span(?: itemprop="duration")? datetime="PT[^"]+">([^<]*)</span>)");
    if (rx.indexIn(html) != -1) {
        using namespace std::chrono;
        QStringList runtimeStr = rx.cap(1).split(":");
        if (runtimeStr.count() == 3) {
            minutes runtime = hours(runtimeStr.at(0).toInt()) + minutes(runtimeStr.at(1).toInt());
            m_movie->setRuntime(runtime);

        } else if (runtimeStr.count() == 2) {
            minutes runtime = minutes(runtimeStr.at(0).toInt());
            m_movie->setRuntime(runtime);
        }
    }

    rx.setPattern(R"(<span class="video_description"(?: itemprop="description")?>(.*)</span>)");
    if (rx.indexIn(html) != -1) {
        QTextDocument doc;
        doc.setHtml(rx.cap(1));
        m_movie->setOverview(doc.toPlainText().trimmed());
    }

    rx.setPattern(R"rx(data-front="([^"]+)")rx");
    if (rx.indexIn(html) != -1) {
        Poster p;
        p.thumbUrl = rx.cap(1);
        p.originalUrl = rx.cap(1);
        m_movie->images().addPoster(p);
    }

    rx.setPattern(R"rx(data-back="([^"]+)")rx");
    if (rx.indexIn(html) != -1) {
        Poster p;
        p.thumbUrl = rx.cap(1);
        p.originalUrl = rx.cap(1);
        m_movie->images().addBackdrop(p);
    }

    {
        rx.setPattern(
            R"re(<div class="star_wrapper" key="([^"]*)"><img [^>]*/><span(?: itemprop="name")?>([^<]*)</span>)re");
        rx.setMinimal(true);
        int offset = 0;
        while ((offset = rx.indexIn(html, offset)) != -1) {
            offset += rx.matchedLength();
            Actor a;
            a.name = rx.cap(2);
            const auto pictureUrl = rx.cap(1);
            if (!pictureUrl.endsWith("missing_f.gif") && !pictureUrl.endsWith("missing_m.gif")) {
                a.thumb = pictureUrl;
            }
            m_movie->addActor(a);
        }
    }

    {
        rx.setPattern("title=\"Plot Oriented -> ([^\"]+)\"");
        int offset = 0;
        while ((offset = rx.indexIn(html, offset)) != -1) {
            offset += rx.matchedLength();
            m_movie->addGenre(rx.cap(1));
        }
    }

    rx.setPattern(R"re(<strong>Studio:</strong> <a(?: itemprop="url")? href="[^"]*"[\s\t\n]*title="([^"]*)")re");
    if (rx.indexIn(html) != -1) {
        m_movie->addStudio(rx.cap(1));
    }

    rx.setPattern(R"re("director":\[\{"@type":"Person","name":"([^"]+)")re");
    if (rx.indexIn(html) != -1) {
        m_movie->setDirector(rx.cap(1));
    }

    // Title may contain `"` which results in invalid HTML.
    rx.setPattern(R"(<a href="https://www.hotmovies.com/series/[^"]*" title=".*" rel="tag">(.*)</a>)");
    if (rx.indexIn(html) != -1) {
        MovieSet set;
        set.name = rx.cap(1);
        m_movie->setSet(set);
    }
}

} // namespace scraper
} // namespace mediaelch
