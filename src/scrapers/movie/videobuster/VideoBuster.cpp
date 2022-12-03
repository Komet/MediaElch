#include "scrapers/movie/videobuster/VideoBuster.h"

#include "globals/Helper.h"
#include "log/Log.h"
#include "scrapers/movie/videobuster/VideoBusterSearchJob.h"
#include "settings/Settings.h"

#include <QSet>
#include <QTextDocument>

namespace mediaelch {
namespace scraper {

VideoBuster::VideoBuster(QObject* parent) : MovieScraper(parent)
{
    m_meta.identifier = ID;
    m_meta.name = "VideoBuster";
    m_meta.description = tr("VideoBuster is a German movie database.");
    m_meta.website = "https://www.videobuster.de";
    m_meta.termsOfService = "https://www.videobuster.de/agb";
    m_meta.privacyPolicy = "https://www.videobuster.de/datenschutz";
    m_meta.help = "https://www.videobuster.de/helpcenter/";
    m_meta.supportedDetails = {MovieScraperInfo::Title,
        MovieScraperInfo::Released,
        MovieScraperInfo::Countries,
        MovieScraperInfo::Certification,
        MovieScraperInfo::Actors,
        MovieScraperInfo::Studios,
        MovieScraperInfo::Runtime,
        MovieScraperInfo::Rating,
        MovieScraperInfo::Genres,
        MovieScraperInfo::Tagline,
        MovieScraperInfo::Overview,
        MovieScraperInfo::Poster,
        MovieScraperInfo::Backdrop,
        MovieScraperInfo::Tags,
        MovieScraperInfo::Director};
    m_meta.supportedLanguages = {"de"};
    m_meta.defaultLocale = "de";
    m_meta.isAdult = false;
}

const MovieScraper::ScraperMeta& VideoBuster::meta() const
{
    return m_meta;
}

void VideoBuster::initialize()
{
    // VideoBuster requires no initialization.
}

bool VideoBuster::isInitialized() const
{
    // VideoBuster requires no initialization.
    return true;
}

MovieSearchJob* VideoBuster::search(MovieSearchJob::Config config)
{
    return new VideoBusterSearchJob(m_api, std::move(config), this);
}

QSet<MovieScraperInfo> VideoBuster::scraperNativelySupports()
{
    return m_meta.supportedDetails;
}

void VideoBuster::changeLanguage(mediaelch::Locale /*locale*/)
{
    // no-op: Only one language is supported and it is hard-coded.
}

void VideoBuster::loadData(QHash<MovieScraper*, mediaelch::scraper::MovieIdentifier> ids,
    Movie* movie,
    QSet<MovieScraperInfo> infos)
{
    if (ids.isEmpty()) {
        // TODO: Should not happen.
        return;
    }

    m_api.loadMovie(ids.constBegin().value().str(), [movie, infos, this](QString data, ScraperError error) {
        movie->clear(infos);

        if (!error.hasError()) {
            data = replaceEntities(data);
            parseAndAssignInfos(data, movie, infos);

        } else {
            // TODO
            showNetworkError(error);
        }
        movie->controller()->scraperLoadDone(this, error);
    });
}

void VideoBuster::parseAndAssignInfos(const QString& html, Movie* movie, QSet<MovieScraperInfo> infos)
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

bool VideoBuster::hasSettings() const
{
    return false;
}

void VideoBuster::loadSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

void VideoBuster::saveSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

QWidget* VideoBuster::settingsWidget()
{
    return nullptr;
}

/**
 * \brief This function replaces entities with their unicode counterparts
 * \param msg String with entities
 * \return String without entities
 */
QString VideoBuster::replaceEntities(const QString msg)
{
    // not nice but I don't know other methods which don't require the gui module
    QString m = msg;
    m.replace("&#039;", "'");
    return m;
}

} // namespace scraper
} // namespace mediaelch
