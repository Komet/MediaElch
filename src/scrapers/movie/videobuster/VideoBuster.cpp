#include "scrapers/movie/videobuster/VideoBuster.h"

#include <QTextDocument>

#include "data/Storage.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "network/NetworkRequest.h"
#include "settings/Settings.h"

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
    // no-op
    // VideoBuster requires no initialization.
}

bool VideoBuster::isInitialized() const
{
    // VideoBuster requires no initialization.
    return true;
}

mediaelch::network::NetworkManager* VideoBuster::network()
{
    return &m_network;
}

QSet<MovieScraperInfo> VideoBuster::scraperNativelySupports()
{
    return m_meta.supportedDetails;
}

void VideoBuster::changeLanguage(mediaelch::Locale /*locale*/)
{
    // no-op: Only one language is supported and it is hard-coded.
}

/**
 * \brief Searches for a movie
 * \param searchStr The Movie name/search string
 * \see VideoBuster::searchFinished
 */
void VideoBuster::search(QString searchStr)
{
    m_api.searchForMovie(searchStr, [this](QString data, ScraperError error) {
        if (error.hasError()) {
            qWarning() << "[VideoBuster] Search Error" << error.message << "|" << error.technical;
            emit searchDone({}, error);

        } else {
            data = replaceEntities(data);
            emit searchDone(parseSearch(data), {});
        }
    });
}

QVector<ScraperSearchResult> VideoBuster::parseSearch(QString html)
{
    QVector<ScraperSearchResult> results;

    QRegularExpression rx("<div class=\"infos\"><a href=\"([^\"]*)\" class=\"title\">([^<]*)</a>");
    rx.setPatternOptions(QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator matches = rx.globalMatch(html);

    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();

        ScraperSearchResult result;
        result.name = match.captured(2);
        result.id = match.captured(1);
        results.append(result);
    }
    return results;
}

/**
 * \brief Starts network requests to download infos from VideoBuster
 * \param ids VideoBuster movie ID
 * \param movie Movie object
 * \param infos List of infos to load
 * \see VideoBuster::loadFinished
 */
void VideoBuster::loadData(QHash<MovieScraper*, mediaelch::scraper::MovieIdentifier> ids,
    Movie* movie,
    QSet<MovieScraperInfo> infos)
{
    m_api.loadMovie(ids.values().first().str(), [movie, infos, this](QString data, ScraperError error) {
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

/**
 * \brief Parses HTML data and assigns it to the given movie object
 * \param html HTML data
 * \param movie Movie object
 * \param infos List of infos to load
 */
void VideoBuster::parseAndAssignInfos(const QString& html, Movie* movie, QSet<MovieScraperInfo> infos)
{
    qDebug() << "[VideoBuster] Parse and assign movie details";
    movie->clear(infos);

    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match;
    QRegularExpressionMatchIterator matches;

    QTextDocument doc;

    // Title
    rx.setPattern("<h1 itemprop=\"name\">([^<]*)</h1>");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Title) && match.hasMatch()) {
        movie->setName(match.captured(1).trimmed());
    }

    // Original Title
    rx.setPattern("<label>Originaltitel</label><br><span itemprop=\"alternateName\">(.*)</span>");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Title) && match.hasMatch()) {
        movie->setOriginalName(match.captured(1).trimmed());
    }

    // Year
    rx.setPattern("<span itemprop=\"copyrightYear\">([0-9]*)</span>");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Released) && match.hasMatch()) {
        movie->setReleased(QDate::fromString(match.captured(1).trimmed(), "yyyy"));
    }

    // Country
    if (infos.contains(MovieScraperInfo::Countries)) {
        rx.setPattern(R"(<label>Produktion</label><br><a href="[^"]*">(.*)</a>)");
        matches = rx.globalMatch(html);

        while (matches.hasNext()) {
            match = matches.next();
            movie->addCountry(helper::mapCountry(match.captured(1).trimmed()));
        }
    }

    // MPAA
    if (infos.contains(MovieScraperInfo::Certification)) {
        // 2016 | FSK 0
        rx.setPattern("[0-9]{4} [|] FSK ([0-9]+)");
        match = rx.match(html);
        if (match.hasMatch()) {
            movie->setCertification(helper::mapCertification(Certification::FSK(match.captured(1))));
        }
    }

    // Actors

    // clear actors
    movie->setActors({});

    if (infos.contains(MovieScraperInfo::Actors)) {
        rx.setPattern("<span itemprop=\"actor\" itemscope itemtype=\"http://schema.org/Person\"><a href=\"[^\"]*\" "
                      "itemprop=\"url\"><span itemprop=\"name\">(.*)</span></a></span>");
        matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            match = matches.next();

            Actor a;
            a.name = match.captured(1).trimmed();
            movie->addActor(a);
        }
    }

    if (infos.contains(MovieScraperInfo::Director)) {
        rx.setPattern("<p><label>Regie</label><br>(.*)</p>");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString contents = match.captured(1);
            QStringList directors;
            rx.setPattern(R"(<a href="/persondtl.php/[^"]*">(.*)</a>)");
            matches = rx.globalMatch(contents);
            while (matches.hasNext()) {
                directors.append(matches.next().captured(1).trimmed());
            }
            movie->setDirector(directors.join(", "));
        }
    }

    if (infos.contains(MovieScraperInfo::Tags)) {
        rx.setPattern("<label>Schlagw&ouml;rter</label><br><span itemprop=\"keywords\">(.*)</span>");
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
    rx.setPattern("<label>Studio</label><br><span itemprop=\"publisher\" itemscope "
                  "itemtype=\"http://schema.org/Organization\">.*<span itemprop=\"name\">(.*)</span></a></span>");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Studios) && match.hasMatch()) {
        movie->addStudio(helper::mapStudio(match.captured(1).trimmed()));
    }

    // Runtime
    rx.setPattern("ca. ([0-9]*) Minuten");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Runtime) && match.hasMatch()) {
        movie->setRuntime(std::chrono::minutes(match.captured(1).trimmed().toInt()));
    }

    // Rating
    if (infos.contains(MovieScraperInfo::Rating)) {
        Rating rating;
        rating.source = "VideoBuster";
        rx.setPattern("<span itemprop=\"ratingCount\">([0-9]*)</span>");
        match = rx.match(html);
        if (match.hasMatch()) {
            rating.voteCount = match.captured(1).trimmed().toInt();
        }

        rx.setPattern("<span itemprop=\"ratingValue\">(.*)</span>");
        match = rx.match(html);
        if (match.hasMatch()) {
            rating.rating = match.captured(1).trimmed().replace(".", "").replace(",", ".").toDouble();
        }
        movie->ratings().push_back(rating);
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
    rx.setPattern(R"(<p class="long_name" itemprop="alternativeHeadline">(.*)</p>)");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Tagline) && match.hasMatch()) {
        movie->setTagline(match.captured(1).trimmed());
    }

    // Overview
    rx.setPattern("<p itemprop=\"description\">(.*)</p>");
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
        rx.setPattern("<h3>Poster</h3><ul class=\"gallery_box  posters\">(.*)</ul>");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString contents = match.captured(1);
            rx.setPattern("<a href=\"https://gfx.videobuster.de/archive/([^\"]*)\" data-title=\"[^\"]*\" "
                          "rel=\"gallery_posters\" target=\"_blank\" class=\"image\">");
            matches = rx.globalMatch(contents);
            while (matches.hasNext()) {
                match = matches.next();

                Poster p;
                p.thumbUrl = "https://gfx.videobuster.de/archive/" + match.captured(1);
                p.originalUrl = "https://gfx.videobuster.de/archive/" + match.captured(1);
                movie->images().addPoster(p);
            }
        }
    }

    // Backdrops
    if (infos.contains(MovieScraperInfo::Backdrop)) {
        rx.setPattern("<h3>Szenenbilder</h3><ul class=\"gallery_box  pictures\">(.*)</ul>");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString contents = match.captured(1);

            rx.setPattern("<a href=\"https://gfx.videobuster.de/archive/([^\"]*)\" data-title=\"[^\"]*\" "
                          "rel=\"gallery_pictures\" target=\"_blank\" class=\"image\">");
            matches = rx.globalMatch(contents);
            while (matches.hasNext()) {
                match = matches.next();

                Poster p;
                p.thumbUrl = "https://gfx.videobuster.de/archive/" + match.captured(1);
                p.originalUrl = "https://gfx.videobuster.de/archive/" + match.captured(1);
                movie->images().addBackdrop(p);
            }
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
