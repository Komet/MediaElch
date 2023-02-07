#include "scrapers/movie/videobuster/VideoBusterScrapeJob.h"

#include "data/movie/Movie.h"
#include "globals/Helper.h"
#include "log/Log.h"
#include "scrapers/movie/videobuster/VideoBusterApi.h"
#include "settings/Settings.h"

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
    // TODO
}

void VideoBusterScrapeJob::parseAndAssignInfos(const QString& html, Movie* movie, const QSet<MovieScraperInfo>& infos)
{
    qCDebug(generic) << "[VideoBuster] Parse and assign movie details";
    movie->clear(infos);

    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match;
    QRegularExpressionMatchIterator matches;

    QTextDocument doc;

    // Title
    rx.setPattern(R"(<h1 itemprop="name" class="[^"]+">([^<]*)</h1>)");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Title) && match.hasMatch()) {
        movie->setName(match.captured(1).trimmed());
    }

    // Original Title
    rx.setPattern(R"(<label>Originaltitel:?</label><[^>]+><span itemprop="alternateName">(.*)</span>)");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Title) && match.hasMatch()) {
        movie->setOriginalName(match.captured(1).trimmed());
    }

    // Year
    rx.setPattern(R"(<span itemprop="copyrightYear">(\d+)</span>)");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Released) && match.hasMatch()) {
        movie->setReleased(QDate::fromString(match.captured(1).trimmed(), "yyyy"));
    }

    // Country
    if (infos.contains(MovieScraperInfo::Countries)) {
        QSet<QString> countries; // avoid duplicates
        rx.setPattern(R"(<label>Produktion:?</label><[^>]+><a href="[^"]*">(.*)</a>)");
        matches = rx.globalMatch(html);

        while (matches.hasNext()) {
            match = matches.next();
            countries << helper::mapCountry(match.captured(1).trimmed());
        }

        for (const QString& country : asConst(countries)) {
            movie->addCountry(country);
        }
    }

    // MPAA
    if (infos.contains(MovieScraperInfo::Certification)) {
        // 2016 | FSK 0
        rx.setPattern(R"(\d{4} [|] FSK (\d+))");
        match = rx.match(html);
        if (match.hasMatch()) {
            movie->setCertification(helper::mapCertification(Certification::FSK(match.captured(1))));
        }
    }

    // Actors

    // clear actors
    movie->setActors({});

    if (infos.contains(MovieScraperInfo::Actors)) {
        rx.setPattern(
            R"(<span itemprop="actor" itemscope itemtype="http://schema.org/Person"><a href="[^"]+" itemprop="url" class="actor-name" itemprop="name">(.+)</a>)");
        matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            match = matches.next();

            Actor a;
            a.name = match.captured(1).trimmed();
            movie->addActor(a);
        }
    }

    if (infos.contains(MovieScraperInfo::Director)) {
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
            movie->setDirector(directors.join(", "));
        }
    }

    if (infos.contains(MovieScraperInfo::Tags)) {
        rx.setPattern(R"(<label>Schlagw&ouml;rter:?</label><[^>]+><span itemprop="keywords">(.*)</span>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString contents = match.captured(1);
            rx.setPattern(R"(<a href="/titlesearch.php[^"]*">(.*)</a>)");
            matches = rx.globalMatch(contents);
            while (matches.hasNext()) {
                movie->addTag(matches.next().captured(1).trimmed());
            }
        }
    }

    // Studio
    rx.setPattern(
        R"(<label>Studio:?</label><[^>]+><span itemprop="publisher" itemscope itemtype="http://schema.org/Organization">.+<span itemprop="name">(.+)</span></a></span>)");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Studios) && match.hasMatch()) {
        movie->addStudio(helper::mapStudio(match.captured(1).trimmed()));
    }

    // Runtime
    rx.setPattern(R"(ca. (\d+) Minuten)");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Runtime) && match.hasMatch()) {
        movie->setRuntime(std::chrono::minutes(match.captured(1).trimmed().toInt()));
    }

    // Rating
    if (infos.contains(MovieScraperInfo::Rating)) {
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
        movie->ratings().setOrAddRating(rating);
    }

    // Genres
    if (infos.contains(MovieScraperInfo::Genres)) {
        rx.setPattern(R"(<a href="/genrelist\.php/.*">(.*)</a>)");
        matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            movie->addGenre(helper::mapGenre(matches.next().captured(1).trimmed()));
        }
    }

    // Tagline
    rx.setPattern(R"(<h4 class="long-name" itemprop="alternativeHeadline">(.+)</h4>)");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Tagline) && match.hasMatch()) {
        movie->setTagline(match.captured(1).trimmed());
    }

    // Overview
    rx.setPattern(R"(<p itemprop="description" class="[^"]+">(.*)</p>)");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Overview) && match.hasMatch()) {
        doc.setHtml(match.captured(1).trimmed());
        movie->setOverview(doc.toPlainText());
        if (Settings::instance()->usePlotForOutline()) {
            movie->setOutline(doc.toPlainText());
        }
    }

    // Posters
    if (infos.contains(MovieScraperInfo::Poster)) {
        rx.setPattern(
            R"re(<a href="//gfx.videobuster.de/archive/([^"]+)" rel="item-posters" [^>]+><img [^>]+ src="//gfx.videobuster.de/archive/([^"]+)"[^>]+></a>)re");
        matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            match = matches.next();

            Poster p;
            p.originalUrl = "https://gfx.videobuster.de/archive/" + match.captured(1);
            p.thumbUrl = "https://gfx.videobuster.de/archive/" + match.captured(2);
            movie->images().addPoster(p);
        }
    }

    // Backdrops
    if (infos.contains(MovieScraperInfo::Backdrop)) {
        rx.setPattern(
            R"re(<a href="//gfx.videobuster.de/archive/([^"]+)" rel="item-pictures" [^>]+><img [^>]+ src="//gfx.videobuster.de/archive/([^"]+)"[^>]+></a>)re");
        matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            match = matches.next();

            Poster p;
            p.originalUrl = "https://gfx.videobuster.de/archive/" + match.captured(1);
            p.thumbUrl = "https://gfx.videobuster.de/archive/" + match.captured(2);
            movie->images().addBackdrop(p);
        }
    }
}

} // namespace scraper
} // namespace mediaelch
