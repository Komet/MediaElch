#include "scrapers/movie/videobuster/VideoBusterScrapeJob.h"

#include "data/movie/Movie.h"
#include "globals/Helper.h"
#include "log/Log.h"
#include "scrapers/movie/videobuster/VideoBusterApi.h"

#include <QRegularExpression>
#include <QTextDocument>

namespace mediaelch {
namespace scraper {

VideoBusterScrapeJob::VideoBusterScrapeJob(VideoBusterApi& api, MovieScrapeJob::Config _config, QObject* parent) :
    MovieScrapeJob(std::move(_config), parent), m_api{api}
{
}

void VideoBusterScrapeJob::doStart()
{
    m_api.loadMovie(config().identifier.str(), [this](QString data, ScraperError error) {
        if (!error.hasError()) {
            data = m_api.replaceEntities(data);
            parseAndAssignInfos(data);

        } else {
            setScraperError(error);
        }
        emitFinished();
    });
}

void VideoBusterScrapeJob::parseAndAssignInfos(const QString& html)
{
    qCDebug(generic) << "[VideoBuster] Parse and assign movie details";

    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match;
    QRegularExpressionMatchIterator matches;

    QTextDocument doc;

    // Title
    rx.setPattern(R"(<h1 itemprop="name" class="[^"]+">([^<]*)</h1>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->setTitle(match.captured(1).trimmed());
    }

    // Original Title
    rx.setPattern(R"(<label>Originaltitel:?</label><[^>]+><span itemprop="alternateName">(.*)</span>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->setOriginalTitle(match.captured(1).trimmed());
    }

    // Year
    rx.setPattern(R"(<span itemprop="copyrightYear">(\d+)</span>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->setReleased(QDate::fromString(match.captured(1).trimmed(), "yyyy"));
    }

    // Country
    {
        QSet<QString> countries; // avoid duplicates
        rx.setPattern(R"(<label>Produktion:?</label><[^>]+><a href="[^"]*">(.*)</a>)");
        matches = rx.globalMatch(html);

        while (matches.hasNext()) {
            match = matches.next();
            countries << helper::mapCountry(match.captured(1).trimmed());
        }

        for (const QString& country : asConst(countries)) {
            m_movie->addCountry(country);
        }
    }

    // MPAA
    {
        // 2016 | FSK 0
        rx.setPattern(R"(\d{4} [|] FSK (\d+))");
        match = rx.match(html);
        if (match.hasMatch()) {
            m_movie->setCertification(helper::mapCertification(Certification::FSK(match.captured(1))));
        }
    }

    // Actors

    // clear actors
    m_movie->setActors({});

    {
        rx.setPattern(
            R"(<span itemprop="actor" itemscope itemtype="http://schema.org/Person"><a href="[^"]+" itemprop="url" class="actor-name" itemprop="name">(.+)</a>)");
        matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            match = matches.next();

            Actor a;
            a.name = match.captured(1).trimmed();
            m_movie->addActor(a);
        }
    }

    {
        rx.setPattern("<label>Regie:?</label><div [^>]+>(.*)</div>");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString contents = match.captured(1);
            QStringList directors;
            rx.setPattern(R"(<a href="/persondtl.php/[^"]+">(.+)</a>)");
            matches = rx.globalMatch(contents);
            while (matches.hasNext()) {
                directors.append(matches.next().captured(1).trimmed());
            }
            m_movie->setDirector(directors.join(", "));
        }
    }

    {
        rx.setPattern(R"(<label>Schlagw&ouml;rter:?</label><[^>]+><span itemprop="keywords">(.*)</span>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString contents = match.captured(1);
            rx.setPattern(R"(<a href="/titlesearch.php[^"]*">(.*)</a>)");
            matches = rx.globalMatch(contents);
            while (matches.hasNext()) {
                m_movie->addTag(matches.next().captured(1).trimmed());
            }
        }
    }

    // Studio
    rx.setPattern(
        R"(<label>Studio:?</label><[^>]+><span itemprop="publisher" itemscope itemtype="http://schema.org/Organization">.+<span itemprop="name">(.+)</span></a></span>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->addStudio(helper::mapStudio(match.captured(1).trimmed()));
    }

    // Runtime
    rx.setPattern(R"(ca. (\d+) Minuten)");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->setRuntime(std::chrono::minutes(match.captured(1).trimmed().toInt()));
    }

    // Rating
    {
        Rating rating;
        rating.source = "VideoBuster";
        rx.setPattern(R"(<span itemprop="ratingCount">(\d+)</span>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            rating.voteCount = match.captured(1).trimmed().toInt();
        }

        rx.setPattern("<span itemprop=\"ratingValue\">(.*)</span>");
        match = rx.match(html);
        if (match.hasMatch()) {
            rating.rating = match.captured(1).trimmed().replace(".", "").replace(",", ".").toDouble();
        }
        m_movie->ratings().setOrAddRating(rating);
    }

    // Genres
    {
        rx.setPattern(R"(<a href="/genrelist\.php/.*">(.*)</a>)");
        matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            m_movie->addGenre(helper::mapGenre(matches.next().captured(1).trimmed()));
        }
    }

    // Tagline
    rx.setPattern(R"(<h4 class="long-name" itemprop="alternativeHeadline">(.+)</h4>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->setTagline(match.captured(1).trimmed());
    }

    // Overview
    rx.setPattern(R"(<p itemprop="description" class="[^"]+">(.*)</p>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        doc.setHtml(match.captured(1).trimmed());
        m_movie->setOverview(doc.toPlainText());
    }

    // Posters
    {
        rx.setPattern(
            R"re(<a href="//gfx.videobuster.de/archive/([^"]+)" rel="item-posters" [^>]+><img [^>]+ src="//gfx.videobuster.de/archive/([^"]+)"[^>]+></a>)re");
        matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            match = matches.next();

            Poster p;
            p.originalUrl = "https://gfx.videobuster.de/archive/" + match.captured(1);
            p.thumbUrl = "https://gfx.videobuster.de/archive/" + match.captured(2);
            m_movie->images().addPoster(p);
        }
    }

    // Backdrops
    {
        rx.setPattern(
            R"re(<a href="//gfx.videobuster.de/archive/([^"]+)" rel="item-pictures" [^>]+><img [^>]+ src="//gfx.videobuster.de/archive/([^"]+)"[^>]+></a>)re");
        matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            match = matches.next();

            Poster p;
            p.originalUrl = "https://gfx.videobuster.de/archive/" + match.captured(1);
            p.thumbUrl = "https://gfx.videobuster.de/archive/" + match.captured(2);
            m_movie->images().addBackdrop(p);
        }
    }
}

} // namespace scraper
} // namespace mediaelch
