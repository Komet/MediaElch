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

/**
 * \brief Parses the search results
 * \param html Downloaded HTML data
 * \return List of search results
 */
QVector<ScraperSearchResult> VideoBuster::parseSearch(QString html)
{
    QVector<ScraperSearchResult> results;
    int pos = 0;
    QRegExp rx("<div class=\"infos\"><a href=\"([^\"]*)\" class=\"title\">([^<]*)</a>");
    rx.setMinimal(true);
    while ((pos = rx.indexIn(html, pos)) != -1) {
        ScraperSearchResult result;
        result.name = rx.cap(2);
        result.id = rx.cap(1);
        results.append(result);
        pos += rx.matchedLength();
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
void VideoBuster::loadData(QHash<MovieScraper*, QString> ids, Movie* movie, QSet<MovieScraperInfo> infos)
{
    m_api.loadMovie(ids.values().first(), [movie, infos, this](QString data, ScraperError error) {
        movie->clear(infos);

        if (!error.hasError()) {
            data = replaceEntities(data);
            parseAndAssignInfos(data, movie, infos);

        } else {
            // TODO
            showNetworkError(error);
        }
        movie->controller()->scraperLoadDone(this);
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

    QRegExp rx;
    rx.setMinimal(true);

    int pos = 0;
    QTextDocument doc;

    // Title
    rx.setPattern("<h1 itemprop=\"name\">([^<]*)</h1>");
    if (infos.contains(MovieScraperInfo::Title) && rx.indexIn(html) != -1) {
        movie->setName(rx.cap(1).trimmed());
    }

    // Original Title
    rx.setPattern("<label>Originaltitel</label><br><span itemprop=\"alternateName\">(.*)</span>");
    if (infos.contains(MovieScraperInfo::Title) && rx.indexIn(html) != -1) {
        movie->setOriginalName(rx.cap(1).trimmed());
    }

    // Year
    rx.setPattern("<span itemprop=\"copyrightYear\">([0-9]*)</span>");
    if (infos.contains(MovieScraperInfo::Released) && rx.indexIn(html) != -1) {
        movie->setReleased(QDate::fromString(rx.cap(1).trimmed(), "yyyy"));
    }

    // Country
    pos = 0;
    rx.setPattern(R"(<label>Produktion</label><br><a href="[^"]*">(.*)</a>)");
    while (infos.contains(MovieScraperInfo::Countries) && (pos = rx.indexIn(html, pos)) != -1) {
        movie->addCountry(helper::mapCountry(rx.cap(1).trimmed()));
        pos += rx.matchedLength();
    }

    // MPAA
    if (infos.contains(MovieScraperInfo::Certification)) {
        // 2016 | FSK 0
        rx.setPattern("[0-9]{4} [|] FSK ([0-9]+)");
        if (rx.indexIn(html) != -1) {
            movie->setCertification(helper::mapCertification(Certification::FSK(rx.cap(1))));
        }
    }

    // Actors
    pos = 0;

    // clear actors
    movie->setActors({});

    rx.setPattern("<span itemprop=\"actor\" itemscope itemtype=\"http://schema.org/Person\"><a href=\"[^\"]*\" "
                  "itemprop=\"url\"><span itemprop=\"name\">(.*)</span></a></span>");
    while (infos.contains(MovieScraperInfo::Actors) && (pos = rx.indexIn(html, pos)) != -1) {
        Actor a;
        a.name = rx.cap(1).trimmed();
        movie->addActor(a);
        pos += rx.matchedLength();
    }

    if (infos.contains(MovieScraperInfo::Director)) {
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
            movie->setDirector(directors.join(", "));
        }
    }

    if (infos.contains(MovieScraperInfo::Tags)) {
        rx.setPattern("<label>Schlagw&ouml;rter</label><br><span itemprop=\"keywords\">(.*)</span>");
        if (rx.indexIn(html) != -1) {
            pos = 0;
            QString contents = rx.cap(1);
            rx.setPattern(R"(<a href="/titlesearch.php[^"]*">(.*)</a>)");
            while ((pos = rx.indexIn(contents, pos)) != -1) {
                movie->addTag(rx.cap(1).trimmed());
                pos += rx.matchedLength();
            }
        }
    }

    // Studio
    rx.setPattern("<label>Studio</label><br><span itemprop=\"publisher\" itemscope "
                  "itemtype=\"http://schema.org/Organization\">.*<span itemprop=\"name\">(.*)</span></a></span>");
    if (infos.contains(MovieScraperInfo::Studios) && rx.indexIn(html) != -1) {
        movie->addStudio(helper::mapStudio(rx.cap(1).trimmed()));
    }

    // Runtime
    rx.setPattern("ca. ([0-9]*) Minuten");
    if (infos.contains(MovieScraperInfo::Runtime) && rx.indexIn(html) != -1) {
        movie->setRuntime(std::chrono::minutes(rx.cap(1).trimmed().toInt()));
    }

    // Rating
    if (infos.contains(MovieScraperInfo::Rating)) {
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
        movie->ratings().push_back(rating);
    }

    // Genres
    if (infos.contains(MovieScraperInfo::Genres)) {
        pos = 0;
        rx.setPattern(R"(<a href="/genrelist\.php/.*">(.*)</a>)");
        while ((pos = rx.indexIn(html, pos)) != -1) {
            movie->addGenre(helper::mapGenre(rx.cap(1).trimmed()));
            pos += rx.matchedLength();
        }
    }

    // Tagline
    rx.setPattern(R"(<p class="long_name" itemprop="alternativeHeadline">(.*)</p>)");
    if (infos.contains(MovieScraperInfo::Tagline) && rx.indexIn(html) != -1) {
        movie->setTagline(rx.cap(1).trimmed());
    }

    // Overview
    rx.setPattern("<p itemprop=\"description\">(.*)</p>");
    if (infos.contains(MovieScraperInfo::Overview) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1).trimmed());
        movie->setOverview(doc.toPlainText());
        if (Settings::instance()->usePlotForOutline()) {
            movie->setOutline(doc.toPlainText());
        }
    }

    // Posters
    if (infos.contains(MovieScraperInfo::Poster)) {
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
                movie->images().addPoster(p);
                pos += rx.matchedLength();
            }
        }
    }

    // Backdrops
    if (infos.contains(MovieScraperInfo::Backdrop)) {
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
                movie->images().addBackdrop(p);
                pos += rx.matchedLength();
            }
        }
    }
}

/**
 * \brief Returns if the scraper has settings
 * \return Scraper has settings
 */
bool VideoBuster::hasSettings() const
{
    return false;
}

/**
 * \brief Loads scrapers settings
 */
void VideoBuster::loadSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

/**
 * \brief Saves scrapers settings
 */
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
