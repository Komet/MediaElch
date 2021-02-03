#include "scrapers/movie/videobuster/VideoBusterScrapeJob.h"

#include "globals/Helper.h"
#include "movies/Movie.h"
#include "scrapers/movie/videobuster/VideoBusterApi.h"

#include <QRegExp>
#include <QTextDocument>

namespace mediaelch {
namespace scraper {

VideoBusterScrapeJob::VideoBusterScrapeJob(VideoBusterApi& api, MovieScrapeJob::Config _config, QObject* parent) :
    MovieScrapeJob(std::move(_config), parent), m_api{api}
{
}

void VideoBusterScrapeJob::execute()
{
    m_api.loadMovie(config().identifier.str(), [this](QString data, ScraperError error) {
        if (!error.hasError()) {
            data = m_api.replaceEntities(data);
            parseAndAssignInfos(data);

        } else {
            m_error = error;
        }
        emit sigFinished(this);
    });
}

void VideoBusterScrapeJob::parseAndAssignInfos(const QString& html)
{
    QRegExp rx;
    rx.setMinimal(true);

    int pos = 0;
    QTextDocument doc;

    // Title
    rx.setPattern("<h1 itemprop=\"name\">([^<]*)</h1>");
    if (rx.indexIn(html) != -1) {
        m_movie->setName(rx.cap(1).trimmed());
    }

    // Original Title
    rx.setPattern("<label>Originaltitel</label><br><span itemprop=\"alternateName\">(.*)</span>");
    if (rx.indexIn(html) != -1) {
        m_movie->setOriginalName(rx.cap(1).trimmed());
    }

    // Year
    rx.setPattern("<span itemprop=\"copyrightYear\">([0-9]*)</span>");
    if (rx.indexIn(html) != -1) {
        m_movie->setReleased(QDate::fromString(rx.cap(1).trimmed(), "yyyy"));
    }

    // Country
    pos = 0;
    rx.setPattern(R"(<label>Produktion</label><br><a href="[^"]*">(.*)</a>)");
    while ((pos = rx.indexIn(html, pos)) != -1) {
        m_movie->addCountry(helper::mapCountry(rx.cap(1).trimmed()));
        pos += rx.matchedLength();
    }

    // MPAA
    {
        // 2016 | FSK 0
        rx.setPattern("[0-9]{4} [|] FSK ([0-9]+)");
        if (rx.indexIn(html) != -1) {
            m_movie->setCertification(helper::mapCertification(Certification::FSK(rx.cap(1))));
        }
    }

    // Actors
    pos = 0;

    rx.setPattern("<span itemprop=\"actor\" itemscope itemtype=\"http://schema.org/Person\"><a href=\"[^\"]*\" "
                  "itemprop=\"url\"><span itemprop=\"name\">(.*)</span></a></span>");
    while ((pos = rx.indexIn(html, pos)) != -1) {
        Actor a;
        a.name = rx.cap(1).trimmed();
        m_movie->addActor(a);
        pos += rx.matchedLength();
    }

    {
        rx.setPattern("<p><label>Regie</label><br>(.*)</p>");
        if (rx.indexIn(html) != -1) {
            pos = 0;
            QString contents = rx.cap(1);
            QStringList directors;
            rx.setPattern(R"(<a href="/persondtl.php/[^"]*">(.*)</a>)");
            while ((pos = rx.indexIn(contents, pos)) != -1) {
                directors.append(rx.cap(1).trimmed());
                pos += rx.matchedLength();
            }
            m_movie->setDirector(directors.join(", "));
        }
    }

    {
        rx.setPattern("<label>Schlagw&ouml;rter</label><br><span itemprop=\"keywords\">(.*)</span>");
        if (rx.indexIn(html) != -1) {
            pos = 0;
            QString contents = rx.cap(1);
            rx.setPattern(R"(<a href="/titlesearch.php[^"]*">(.*)</a>)");
            while ((pos = rx.indexIn(contents, pos)) != -1) {
                m_movie->addTag(rx.cap(1).trimmed());
                pos += rx.matchedLength();
            }
        }
    }

    // Studio
    rx.setPattern("<label>Studio</label><br><span itemprop=\"publisher\" itemscope "
                  "itemtype=\"http://schema.org/Organization\">.*<span itemprop=\"name\">(.*)</span></a></span>");
    if (rx.indexIn(html) != -1) {
        m_movie->addStudio(helper::mapStudio(rx.cap(1).trimmed()));
    }

    // Runtime
    rx.setPattern("ca. ([0-9]*) Minuten");
    if (rx.indexIn(html) != -1) {
        m_movie->setRuntime(std::chrono::minutes(rx.cap(1).trimmed().toInt()));
    }

    // Rating
    {
        Rating rating;
        rating.source = "VideoBuster";
        rx.setPattern("<span itemprop=\"ratingCount\">([0-9]*)</span>");
        if (rx.indexIn(html) != -1) {
            rating.voteCount = rx.cap(1).trimmed().toInt();
        }
        rx.setPattern("<span itemprop=\"ratingValue\">(.*)</span>");
        if (rx.indexIn(html) != -1) {
            rating.rating = rx.cap(1).trimmed().replace(".", "").replace(",", ".").toDouble();
        }
        m_movie->ratings().push_back(rating);
    }

    // Genres
    {
        pos = 0;
        rx.setPattern(R"(<a href="/genrelist\.php/.*">(.*)</a>)");
        while ((pos = rx.indexIn(html, pos)) != -1) {
            m_movie->addGenre(helper::mapGenre(rx.cap(1).trimmed()));
            pos += rx.matchedLength();
        }
    }

    // Tagline
    rx.setPattern(R"(<p class="long_name" itemprop="alternativeHeadline">(.*)</p>)");
    if (rx.indexIn(html) != -1) {
        m_movie->setTagline(rx.cap(1).trimmed());
    }

    // Overview
    rx.setPattern("<p itemprop=\"description\">(.*)</p>");
    if (rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1).trimmed());
        m_movie->setOverview(doc.toPlainText());
    }

    // Posters
    {
        rx.setPattern("<h3>Poster</h3><ul class=\"gallery_box  posters\">(.*)</ul>");
        if (rx.indexIn(html) != -1) {
            QString contents = rx.cap(1);
            pos = 0;
            rx.setPattern("<a href=\"https://gfx.videobuster.de/archive/([^\"]*)\" data-title=\"[^\"]*\" "
                          "rel=\"gallery_posters\" target=\"_blank\" class=\"image\">");
            while ((pos = rx.indexIn(contents, pos)) != -1) {
                Poster p;
                p.thumbUrl = "https://gfx.videobuster.de/archive/" + rx.cap(1);
                p.originalUrl = "https://gfx.videobuster.de/archive/" + rx.cap(1);
                m_movie->images().addPoster(p);
                pos += rx.matchedLength();
            }
        }
    }

    // Backdrops
    {
        rx.setPattern("<h3>Szenenbilder</h3><ul class=\"gallery_box  pictures\">(.*)</ul>");
        if (rx.indexIn(html) != -1) {
            QString contents = rx.cap(1);
            pos = 0;
            rx.setPattern("<a href=\"https://gfx.videobuster.de/archive/([^\"]*)\" data-title=\"[^\"]*\" "
                          "rel=\"gallery_pictures\" target=\"_blank\" class=\"image\">");
            while ((pos = rx.indexIn(contents, pos)) != -1) {
                Poster p;
                p.thumbUrl = "https://gfx.videobuster.de/archive/" + rx.cap(1);
                p.originalUrl = "https://gfx.videobuster.de/archive/" + rx.cap(1);
                m_movie->images().addBackdrop(p);
                pos += rx.matchedLength();
            }
        }
    }
}

} // namespace scraper
} // namespace mediaelch
