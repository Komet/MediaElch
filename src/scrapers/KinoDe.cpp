#include "KinoDe.h"

#include <QTextDocument>
#include <QWidget>

#include "data/Storage.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/NetworkReplyWatcher.h"
#include "settings/Settings.h"

/**
 * @brief KinoDe::KinoDe
 * @param parent
 */
KinoDe::KinoDe(QObject *parent)
{
    setParent(parent);
    m_scraperSupports << MovieScraperInfos::Title         //
                      << MovieScraperInfos::Genres        //
                      << MovieScraperInfos::Released      //
                      << MovieScraperInfos::Countries     //
                      << MovieScraperInfos::Certification //
                      << MovieScraperInfos::Runtime       //
                      << MovieScraperInfos::Overview      //
                      << MovieScraperInfos::Backdrop      //
                      << MovieScraperInfos::Poster;
    // Actor, director and writer parsing does not work at the moment.
    // KinoDe does not provide enough information that we can parse.
    // We either have cases where the director is recognized as an
    // actor and vice versa.
    // << MovieScraperInfos::Actors     //
    // << MovieScraperInfos::Director      //
    // << MovieScraperInfos::Writer;
}

/**
 * @brief Returns the name of the scraper
 * @return Name of the Scraper
 */
QString KinoDe::name()
{
    return QStringLiteral("Kino.de");
}

QString KinoDe::identifier()
{
    return QStringLiteral("kinode");
}

bool KinoDe::isAdult()
{
    return false;
}

/**
 * @brief Returns if the scraper has settings
 * @return Scraper has settings
 */
bool KinoDe::hasSettings()
{
    return false;
}

QWidget *KinoDe::settingsWidget()
{
    return nullptr;
}

/**
 * @brief Loads scrapers settings
 */
void KinoDe::loadSettings(QSettings &settings)
{
    Q_UNUSED(settings);
}

/**
 * @brief Saves scrapers settings
 */
void KinoDe::saveSettings(QSettings &settings)
{
    Q_UNUSED(settings);
}

/**
 * @brief Returns a list of infos available from the scraper
 * @return List of supported infos
 */
QList<MovieScraperInfos> KinoDe::scraperSupports()
{
    return m_scraperSupports;
}

QList<MovieScraperInfos> KinoDe::scraperNativelySupports()
{
    return m_scraperSupports;
}

std::vector<ScraperLanguage> KinoDe::supportedLanguages()
{
    return {{tr("German"), "de"}};
}

void KinoDe::changeLanguage(QString /*languageKey*/)
{
    // no-op: Only one language is supported and it is hard-coded.
}

QString KinoDe::defaultLanguageKey()
{
    return QStringLiteral("de");
}

/**
 * @brief Searches for a movie
 * @param searchStr The Movie name/search string
 * @see KinoDe::searchFinished
 */
void KinoDe::search(QString searchStr)
{
    qDebug() << "Entered, movie searchStr = " << searchStr;
    QString encodedSearch = Helper::instance()->toLatin1PercentEncoding(searchStr);
    QUrl url{QStringLiteral("https://www.kino.de/se/?searchterm=%1&types=movie").arg(encodedSearch)};
    QNetworkReply *const reply = m_qnam.get(QNetworkRequest{url});
    new NetworkReplyWatcher(this, reply);
    connect(reply, &QNetworkReply::finished, this, &KinoDe::searchFinished);
}

/**
 * @brief Called when the search result was downloaded.
 *        Emits "searchDone" after parsing the search result page.
 * @see KinoDe::parseSearch
 */
void KinoDe::searchFinished()
{
    auto reply = static_cast<QNetworkReply *const>(QObject::sender());

    QList<ScraperSearchResult> results;
    if (reply->error() == QNetworkReply::NoError) {
        const QString html = QString::fromUtf8(reply->readAll());
        results = parseSearch(html);

    } else {
        qWarning() << "Network Error:" << reply->errorString();
    }

    reply->deleteLater();
    emit searchDone(results);
}

/**
 * @brief Parses the search results.
 * @param html Downloaded HTML data.
 * @return List of search results.
 */
QList<ScraperSearchResult> KinoDe::parseSearch(const QString &html)
{
    QList<ScraperSearchResult> results;
    QRegExp rxTitle(R"rx(<a class="[^"]*" href="(?:https:)?//www.kino.de/film/([^"/]*)/">([^<]*)</a>)rx");
    rxTitle.setMinimal(true);

    int pos = 0;
    while ((pos = rxTitle.indexIn(html, pos)) != -1) {
        ScraperSearchResult result;
        result.id = rxTitle.cap(1);
        result.name = rxTitle.cap(2);

        // Get year from the URL
        QRegExp rxYear(R"([-.a-z1-9]+([0-9]{4}))");

        rxYear.setMinimal(true);
        if (rxYear.indexIn(result.id) != -1) {
            result.released = QDate::fromString(rxYear.cap(1), "yyyy");
        }

        results.append(result);
        pos += rxTitle.matchedLength();
    }

    return results;
}

/**
 * @brief Starts network requests to download infos from Kino.de
 * @param id KinoDe movie ID
 * @param movie Movie object
 * @param infos List of infos to load
 * @see KinoDe::loadFinished
 */
void KinoDe::loadData(QMap<ScraperInterface *, QString> ids, Movie *movie, QList<MovieScraperInfos> infos)
{
    movie->clear(infos);
    const QString &movieSlug = ids.values().first();
    const QUrl url{QStringLiteral("https://www.kino.de/film/%1/").arg(movieSlug)};
    QNetworkReply *const reply = m_qnam.get(QNetworkRequest{url});
    new NetworkReplyWatcher(this, reply);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, &QNetworkReply::finished, this, &KinoDe::loadFinished);
}

/**
 * @brief Called when the movie infos are downloaded.
 * @see KinoDe::parseAndAssignInfos
 */
void KinoDe::loadFinished()
{
    auto reply = static_cast<QNetworkReply *const>(QObject::sender());
    Movie *const movie = reply->property("storage").value<Storage *>()->movie();
    const QList<MovieScraperInfos> infos = reply->property("infosToLoad").value<Storage *>()->movieInfosToLoad();
    reply->deleteLater();

    if (movie == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QString html = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(html, *movie, infos);
        // parseAndAssignActors(html, *movie, infos);
        parseAndAssignImages(html, *movie, infos);

    } else {
        qWarning() << "Network Error:" << reply->errorString();
    }

    movie->controller()->scraperLoadDone(this);
}

/**
 * @brief Parses HTML data and assigns it to the given movie object.
 * @param html HTML data
 * @param movie Movie object
 * @param infos List of infos to load
 */
void KinoDe::parseAndAssignInfos(const QString &html, Movie &movie, const QList<MovieScraperInfos> &infos)
{
    QRegExp rx;
    rx.setMinimal(true);
    QTextDocument doc;

    // Title
    rx.setPattern(R"(<h1 class="smb-article-title"[^>]*>(.*)</h1>)");
    if (infos.contains(MovieScraperInfos::Title) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1).trimmed());
        movie.setName(doc.toPlainText());
    }

    // Genre
    if (infos.contains(MovieScraperInfos::Genres)) {
        rx.setPattern(R"(<dt>Genre</dt>\n? *<dd>((<a href="[^"]+/">[^<]+</a>(<span>, </span>)?)+)</dd>)");

        if (rx.indexIn(html) != -1) {
            const auto genres = rx.cap(1).split("<span>, </span>");
            for (const auto &genre : genres) {
                rx.setPattern(R"(<a href="https://www.kino.de/filme/genres/[^"]+/">([^<]+)</a>)");
                if (rx.indexIn(genre) != -1) {
                    doc.setHtml(rx.cap(1).trimmed());
                    movie.addGenre(doc.toPlainText());
                }
            }
        }
    }

    // Year
    rx.setPattern(R"(<dd class="startdate"><time datetime="([0-9]{4}.[0-9]{2}.[0-9]{2})*">)");
    if (infos.contains(MovieScraperInfos::Released) && rx.indexIn(html) != -1) {
        movie.setReleased(QDate::fromString(rx.cap(1).trimmed(), Qt::ISODate));
    }

    // Country
    rx.setPattern(R"(<dt>Produktionsland</dt>\n? *<dd><a href="https://www.kino.de/filme/laender/[^"]+">(.*)</a>)");
    if (infos.contains(MovieScraperInfos::Countries) && rx.indexIn(html) != -1) {
        movie.addCountry(Helper::instance()->mapCountry(rx.cap(1).trimmed()));
    }

    // MPAA
    rx.setPattern("<dt>FSK</dt>\n? *<dd><a href=\"https://www.kino.de/filme/fsk/[^\"]+/\">ab ([0-9]+)</a></dd>");
    if (infos.contains(MovieScraperInfos::Certification) && rx.indexIn(html) != -1) {
        movie.setCertification(Helper::instance()->mapCertification(Certification("FSK " + rx.cap(1))));
    }

    // Runtime
    rx.setPattern(R"(<dt class="length">Dauer</dt>\n? *<dd class="length">([0-9]+) Min</dd>)");
    if (infos.contains(MovieScraperInfos::Runtime) && rx.indexIn(html) != -1) {
        movie.setRuntime(std::chrono::minutes(rx.cap(1).trimmed().toInt()));
    }

    if (infos.contains(MovieScraperInfos::Overview)) {
        // Overview
        // Note: kino.de's HTML is broken. This might change in the future.
        rx.setPattern(R"(<p class="movie-plot-synopsis">(.+)</p>)");
        if (rx.indexIn(html) != -1) {
            doc.setHtml(rx.cap(1).trimmed());
            movie.setOutline(doc.toPlainText());
        }

        rx.setPattern(R"(<p class="movie-plot-synopsis">.+</p>\n *<p> +<p>(.*)</p>)");
        if (rx.indexIn(html) != -1) {
            doc.setHtml(rx.cap(1).trimmed());
            movie.setOverview(doc.toPlainText());
        }

        if (movie.outline().isEmpty() && Settings::instance()->usePlotForOutline()) {
            movie.setOutline(movie.overview());
        }
    }
}

/**
 * @brief Parses HTML data and assigns image URLs (poster + backdrops) to the given movie object.
 * @param html HTML data
 * @param movie Movie object
 * @param infos List of infos to load
 */
void KinoDe::parseAndAssignImages(const QString &html, Movie &movie, const QList<MovieScraperInfos> &infos)
{
    if (infos.contains(MovieScraperInfos::Poster)) {
        parsePoster(html, movie);
    }
    if (infos.contains(MovieScraperInfos::Backdrop)) {
        parseBackdrops(html, movie);
    }
}

/**
 * @brief Parses HTML data and assigns actors to the given movie object.
 * @param html HTML data
 * @param movie Movie object
 * @param infos List of infos to load
 */
void KinoDe::parseAndAssignActors(const QString &html, Movie &movie, const QList<MovieScraperInfos> &infos)
{
    QRegExp rx;
    rx.setMinimal(true);

    if (infos.contains(MovieScraperInfos::Director)) {
        rx.setPattern("<dt>\\s*<small>Regisseur</small>(.*)</dt>");
        if (rx.indexIn(html) != 1) {
            QTextDocument doc(rx.cap(1).trimmed());
            movie.setDirector(doc.toPlainText());
        }
    }

    if (infos.contains(MovieScraperInfos::Writer)) {
        rx.setPattern("<dt>\\s*<small>Drehbuch</small>(.*)</dt>");
        if (rx.indexIn(html) != 1) {
            movie.setWriter(rx.cap(1).trimmed());
        }
    }

    if (infos.contains(MovieScraperInfos::Actors)) {
        // Really ugly regex. We may want to switch to DOM parsing...
        // "src" attribute may either be "src" or "data-pagespeed-lazy-src"
        rx.setPattern(R"re(<a href="https://www.kino.de/star/[^"]+">\n +<figure>\n +<div class="[_A-z -]*">\n +<img
    class="product-slide-no-image" [A-z-]*src="([^"]+)"[^>]+/>\n.*</div>\n\n *\n.*\n +<div
    class="product-slide-headline">\n +([^<]+)</div>)re");

        int pos = 0;
        qWarning() << rx.indexIn(html, pos);
        while ((pos = rx.indexIn(html, pos)) != -1) {
            Actor a;
            a.thumb = !rx.cap(1).contains("platzhalter") ? rx.cap(1) : "";
            a.name = rx.cap(2).trimmed();

            QTextDocument doc(rx.cap(3).replace("&bdquo;", "").replace("&ldquo;", "").trimmed());
            a.role = doc.toPlainText();

            movie.addActor(a);
            pos += rx.matchedLength();
        }
    }
}

/**
 * @brief Parses HTML data and assigns the poster to the given movie.
 * @param html HTML data
 * @param movie Movie object
 */
void KinoDe::parsePoster(const QString &html, Movie &movie)
{
    QRegExp rx(R"re(<div class="product-meta product-meta-movie[^"]*" [^>]*>\s*<figure>\s*<img [^"]*src="([^"]*)")re");
    rx.setMinimal(true);

    if (rx.indexIn(html) != -1 && !rx.cap(1).contains("platzhalter")) {
        Poster p;
        p.thumbUrl = rx.cap(1);
        p.originalUrl = rx.cap(1);
        movie.images().addPoster(p);
    }
}

/**
 * @brief Parses HTML data and assigns backgrounds to the given movie.
 * @param html HTML data
 * @param movie Movie object
 */
void KinoDe::parseBackdrops(const QString &html, Movie &movie)
{
    // Get list items with the background images.
    // The "data-lb-item" attribute contains URL encoded JSON with thumb- and original image-URL.
    QRegExp rx(
        R"re(<li class="slide-item lightbox-gallery-image" data-action="gallery-lightbox" data-lb-item="([^"]*)")re");
    rx.setMinimal(true);

    const auto normalizeUrl = [](QString urlStr) {
        // Kino.de escapes / to \/ even though it's not necessary.
        urlStr.replace(R"(\/)", "/");
        // Some image URLs start with "//".
        if (urlStr.startsWith("//")) {
            urlStr = "https:" + urlStr;
        }
        return urlStr;
    };

    int pos = 0;
    while ((pos = rx.indexIn(html, pos)) != -1) {
        const auto str = QUrl::fromPercentEncoding(rx.cap(1).toLocal8Bit());
        Poster p;

        // Get the background thumb
        QRegExp thumbRx(R"re("thumb":"([^"]+)")re");
        thumbRx.setMinimal(true);

        if (thumbRx.indexIn(str) != -1) {
            p.thumbUrl = normalizeUrl(thumbRx.cap(1));
        }

        // Get the background full image url
        QRegExp originalRx(R"re("content":"<img [^"]*src=\\"([^"]+)\\")re");
        originalRx.setMinimal(true);

        if (originalRx.indexIn(str) != -1) {
            p.originalUrl = normalizeUrl(originalRx.cap(1));
        }
        if (!p.originalUrl.isEmpty() || !p.thumbUrl.isEmpty()) {
            movie.images().addBackdrop(p);
        }

        pos += rx.matchedLength();
    }
}
