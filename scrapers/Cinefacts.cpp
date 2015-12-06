#include "Cinefacts.h"
#include <QTextDocument>
#include <QWidget>
#include "data/Storage.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/NetworkReplyWatcher.h"
#include "settings/Settings.h"

/**
 * @brief Cinefacts::Cinefacts
 * @param parent
 */
Cinefacts::Cinefacts(QObject *parent)
{
    setParent(parent);
    m_scraperSupports << MovieScraperInfos::Title
                      << MovieScraperInfos::Genres
                      << MovieScraperInfos::Released
                      << MovieScraperInfos::Countries
                      << MovieScraperInfos::Actors
                      << MovieScraperInfos::Studios
                      << MovieScraperInfos::Certification
                      << MovieScraperInfos::Runtime
                      << MovieScraperInfos::Overview
                      << MovieScraperInfos::Backdrop
                      << MovieScraperInfos::Poster
                      << MovieScraperInfos::Director
                      << MovieScraperInfos::Writer;
}

/**
 * @brief Returns the name of the scraper
 * @return Name of the Scraper
 */
QString Cinefacts::name()
{
    return QString("Cinefacts");
}

QString Cinefacts::identifier()
{
    return QString("cinefacts");
}

bool Cinefacts::isAdult()
{
    return false;
}

/**
 * @brief Returns if the scraper has settings
 * @return Scraper has settings
 */
bool Cinefacts::hasSettings()
{
    return false;
}

QWidget *Cinefacts::settingsWidget()
{
    return 0;
}

/**
 * @brief Loads scrapers settings
 */
void Cinefacts::loadSettings(QSettings &settings)
{
    Q_UNUSED(settings);
}

/**
 * @brief Saves scrapers settings
 */
void Cinefacts::saveSettings(QSettings &settings)
{
    Q_UNUSED(settings);
}

/**
 * @brief Just returns a pointer to the scrapers network access manager
 * @return Network Access Manager
 */
QNetworkAccessManager *Cinefacts::qnam()
{
    return &m_qnam;
}

/**
 * @brief Returns a list of infos available from the scraper
 * @return List of supported infos
 */
QList<int> Cinefacts::scraperSupports()
{
    return m_scraperSupports;
}

QList<int> Cinefacts::scraperNativelySupports()
{
    return m_scraperSupports;
}

/**
 * @brief Searches for a movie
 * @param searchStr The Movie name/search string
 * @see Cinefacts::searchFinished
 */
void Cinefacts::search(QString searchStr)
{
    qDebug() << "Entered, searchStr=" << searchStr;
    QString encodedSearch = Helper::instance()->toLatin1PercentEncoding(searchStr);
    QUrl url(QString("http://www.cinefacts.de/search/site/q/%1/").arg(encodedSearch).toUtf8());
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    new NetworkReplyWatcher(this, reply);
    connect(reply, SIGNAL(finished()), this, SLOT(searchFinished()));
}

/**
 * @brief Called when the search result was downloaded
 *        Emits "searchDone" if there are no more pages in the result set
 * @see Cinefacts::parseSearch
 */
void Cinefacts::searchFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();

    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 302 ||
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 301) {
        qDebug() << "Got redirect" << reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
        if (!redirect.startsWith("http"))
            redirect.prepend("http://www.cinefacts.de");
        QUrl url(redirect);
        QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
        new NetworkReplyWatcher(this, reply);
        connect(reply, SIGNAL(finished()), this, SLOT(searchFinished()));
        return;
    }

    QList<ScraperSearchResult> results;
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        results = parseSearch(msg);
    } else {
        qWarning() << "Network Error" << reply->errorString();
    }
    emit searchDone(results);
}

/**
 * @brief Parses the search results
 * @param html Downloaded HTML data
 * @return List of search results
 */
QList<ScraperSearchResult> Cinefacts::parseSearch(QString html)
{
    QList<ScraperSearchResult> results;
    int pos = 0;
    QRegExp rx("<a class=\"s_link\" href=\"/Filme/([^\"]*)\">([^<]*)</a>");
    rx.setMinimal(true);
    while ((pos = rx.indexIn(html, pos)) != -1) {
        ScraperSearchResult result;
        result.id = rx.cap(1);

        QRegExp rx2("(.*) \\[([0-9]{4})\\]");
        rx2.setMinimal(true);
        if (rx2.indexIn(rx.cap(2)) != -1) {
            result.name = rx2.cap(1);
            result.released = QDate::fromString(rx2.cap(2), "yyyy");
        } else {
            result.name = rx.cap(2);
        }

        results.append(result);
        pos += rx.matchedLength();
    }
    return results;
}

/**
 * @brief Starts network requests to download infos from Cinefacts
 * @param id Cinefacts movie ID
 * @param movie Movie object
 * @param infos List of infos to load
 * @see Cinefacts::loadFinished
 */
void Cinefacts::loadData(QMap<ScraperInterface*, QString> ids, Movie *movie, QList<int> infos)
{
    movie->clear(infos);

    QUrl url(QString("http://www.cinefacts.de/Filme/%1").arg(ids.values().first()));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    new NetworkReplyWatcher(this, reply);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("cinefactsId", ids.values().first());
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(loadFinished()));
}

/**
 * @brief Called when the movie infos are downloaded
 * @see Cinefacts::parseAndAssignInfos
 */
void Cinefacts::loadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    QString cinefactsId = reply->property("cinefactsId").toString();
    reply->deleteLater();
    if (!movie)
        return;

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, reply->property("infosToLoad").value<Storage*>()->infosToLoad());
        reply = qnam()->get(QNetworkRequest(QUrl(QString("http://www.cinefacts.de/Filme/%1/Besetzung-Stab/").arg(cinefactsId))));
        reply->setProperty("storage", Storage::toVariant(reply, movie));
        reply->setProperty("cinefactsId", cinefactsId);
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, SIGNAL(finished()), this, SLOT(actorsFinished()));
    } else {
        qWarning() << "Network Error" << reply->errorString();
        movie->controller()->scraperLoadDone(this);
    }
}

void Cinefacts::actorsFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    QString cinefactsId = reply->property("cinefactsId").toString();
    reply->deleteLater();
    if (!movie)
        return;

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignActors(msg, movie, reply->property("infosToLoad").value<Storage*>()->infosToLoad());
        reply = qnam()->get(QNetworkRequest(QUrl(QString("http://www.cinefacts.de/Filme/%1/Bildergalerie/").arg(cinefactsId))));
        reply->setProperty("storage", Storage::toVariant(reply, movie));
        reply->setProperty("cinefactsId", cinefactsId);
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, SIGNAL(finished()), this, SLOT(imagesFinished()));
    } else {
        qWarning() << "Network Error" << reply->errorString();
        movie->controller()->scraperLoadDone(this);
    }
}

void Cinefacts::imagesFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!movie)
        return;

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        QStringList posters;
        QStringList backdrops;
        parseImages(msg, posters, backdrops);
        if (posters.count() > 0 && infos.contains(MovieScraperInfos::Poster)) {
            reply = qnam()->get(QNetworkRequest(QUrl(QString("http://www.cinefacts.de%1").arg(posters.takeFirst()))));
            reply->setProperty("storage", Storage::toVariant(reply, movie));
            reply->setProperty("posters", posters);
            reply->setProperty("backdrops", backdrops);
            reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
            connect(reply, SIGNAL(finished()), this, SLOT(posterFinished()));
            return;
        }

        if (posters.count() > 0 && infos.contains(MovieScraperInfos::Backdrop)) {
            reply = qnam()->get(QNetworkRequest(QUrl(QString("http://www.cinefacts.de%1").arg(backdrops.takeFirst()))));
            reply->setProperty("storage", Storage::toVariant(reply, movie));
            reply->setProperty("backdrops", backdrops);
            reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
            connect(reply, SIGNAL(finished()), this, SLOT(backdropFinished()));
            return;
        }
    } else {
        qWarning() << "Network Error" << reply->errorString();
    }
    movie->controller()->scraperLoadDone(this);
}

/**
 * @brief Parses HTML data and assigns it to the given movie object
 * @param html HTML data
 * @param movie Movie object
 * @param infos List of infos to load
 */
void Cinefacts::parseAndAssignInfos(QString html, Movie *movie, QList<int> infos)
{
    QRegExp rx;
    rx.setMinimal(true);
    QTextDocument doc;

    // Title
    rx.setPattern("<span itemprop=\"name\">(.*)</span>");
    if (infos.contains(MovieScraperInfos::Title) && rx.indexIn(html) != -1)
        movie->setName(rx.cap(1).trimmed());

    // Original Title
    rx.setPattern("<span itemprop=\"alternativeHeadline\" >(.*)</span>");
    if (infos.contains(MovieScraperInfos::Title) && rx.indexIn(html) != -1)
        movie->setOriginalName(rx.cap(1).trimmed());

    // Genre
    if (infos.contains(MovieScraperInfos::Genres)) {
        rx.setPattern("<span itemprop=\"genre\" >(.*)</span>");
        int offset = 0;
        while ((offset = rx.indexIn(html, offset)) != -1) {
            offset += rx.matchedLength();
            movie->addGenre(rx.cap(1));
        }
    }

    // Year
    rx.setPattern("<time datetime=\"[^\"]*\" itemprop=\"dateCreated\" >([0-9]{4})</time>");
    if (infos.contains(MovieScraperInfos::Released) && rx.indexIn(html) != -1)
        movie->setReleased(QDate::fromString(rx.cap(1).trimmed(), "yyyy"));

    // Country
    rx.setPattern("<span itemprop=\"genre\" >[^>]*</span> \\| (.*) \\(<time datetime=");
    if (infos.contains(MovieScraperInfos::Countries) && rx.indexIn(html) != -1)
        movie->addCountry(Helper::instance()->mapCountry(rx.cap(1).trimmed()));

    // Studio
    rx.setPattern("<span itemscope itemprop=\"provider\" itemtype=\"http://www.schema.org/Organization\" ><span itemprop=\"name\" >([^<]*)</span>");
    if (infos.contains(MovieScraperInfos::Studios) && rx.indexIn(html) != -1)
        movie->addStudio(Helper::instance()->mapStudio(rx.cap(1).trimmed()));

    // MPAA
    rx.setPattern("Freigegeben ab ([0-9]*) Jahren");
    if (infos.contains(MovieScraperInfos::Certification) && rx.indexIn(html) != -1)
        movie->setCertification(Helper::instance()->mapCertification("FSK " + rx.cap(1)));

    // Runtime
    rx.setPattern("<time itemprop=\"duration\" datetime=\"PT[^\"]*\" >([0-9]*)</time>");
    if (infos.contains(MovieScraperInfos::Runtime) && rx.indexIn(html) != -1)
        movie->setRuntime(rx.cap(1).trimmed().toInt());

    // Overview
    rx.setPattern("<span class=\"thisSummary\" itemprop=\"description\">.*<strong>Inhalt: </strong>(.*)</span>");
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1).trimmed());
        movie->setOverview(doc.toPlainText());
        if (Settings::instance()->usePlotForOutline())
            movie->setOutline(doc.toPlainText());
    }
}

void Cinefacts::parseAndAssignActors(QString html, Movie *movie, QList<int> infos)
{
    QRegExp rx;
    rx.setMinimal(true);
    if (infos.contains(MovieScraperInfos::Director)) {
        rx.setPattern("<h4>Regie</h4></header><div class=\"teasers  teasers_full\"><article><div class=\"item_content\"><header><h5><a href=\"[^\"]*\">(.*)</a></h5>");
        if (rx.indexIn(html) != 1)
            movie->setDirector(rx.cap(1));
    }

    if (infos.contains(MovieScraperInfos::Writer)) {
        rx.setPattern("<h4>Drehbuch</h4></header><div class=\"teasers  teasers_full\"><article><div class=\"item_content\"><header><h5><a href=\"[^\"]*\">(.*)</a>");
        if (rx.indexIn(html) != 1)
            movie->setWriter(rx.cap(1));
    }

    if (infos.contains(MovieScraperInfos::Actors)) {
        rx.setPattern("<section><header><h4>Darsteller</h4></header><div class=\"teasers  teasers_bild\">(.*)</div></section>");
        if (rx.indexIn(html) != -1) {
            QString actors = rx.cap(1);
            QRegExp rx2("<article><figure class=\"item_img\"><a href=\"[^\"]*\"><img  src=\"(.*)\" class=\"thumb\" ></a></figure><div class=\"item_content\"><header><h5><a href=\".*\">(.*)</a><span class=\"right2\"> Rolle: (.*)</span></h5></header>");
            rx2.setMinimal(true);
            int pos = 0;
            while ((pos = rx2.indexIn(actors, pos)) != -1) {
                QString thumb = rx2.cap(1);
                if (!thumb.startsWith("http://"))
                    thumb.prepend("http://www.cinefacts.de");
                Actor a;
                a.name = rx2.cap(2);
                a.role = rx2.cap(3);
                a.thumb = thumb;
                movie->addActor(a);
                pos += rx2.matchedLength();
            }
        }
    }
}

void Cinefacts::parseImages(QString data, QStringList &posters, QStringList &backgrounds)
{
    QRegExp rx("<header><h3>Poster</h3></header>.*<ul>(.*)</ul>");
    rx.setMinimal(true);
    if (rx.indexIn(data) != -1) {
        QString poster = rx.cap(1);
        QRegExp rx2("<li><a href=\"([^\"]*)\">");
        rx2.setMinimal(true);
        int pos = 0;
        while ((pos = rx2.indexIn(poster, pos)) != -1) {
            posters.append(rx2.cap(1));
            pos += rx2.matchedLength();
        }
    }

    rx.setPattern("<header><h3>Szenenbilder</h3></header>.*<ul>(.*)</ul>");
    rx.setMinimal(true);
    if (rx.indexIn(data) != -1) {
        QString background = rx.cap(1);
        QRegExp rx2("<li><a href=\"([^\"]*)\">");
        rx2.setMinimal(true);
        int pos = 0;
        while ((pos = rx2.indexIn(background, pos)) != -1) {
            backgrounds.append(rx2.cap(1));
            pos += rx2.matchedLength();
        }
    }
}


/**
 * @brief Called when poster scraping has finished
 * Starts the next poster download or the backdrop download or tells the movie that scraping is done
 */
void Cinefacts::posterFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    QStringList posters = reply->property("posters").toStringList();
    QStringList backdrops = reply->property("backdrops").toStringList();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    if (!movie)
        return;

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        QRegExp rx("<a href=\"([^\"]*)\" target=\"_blank\">Bild in Originalgr..e</a>");
        rx.setMinimal(true);
        if (rx.indexIn(msg) != -1) {
            Poster p;
            p.thumbUrl = rx.cap(1);
            p.originalUrl = rx.cap(1);
            movie->addPoster(p);
        }

        if (!posters.isEmpty()) {
            reply = qnam()->get(QNetworkRequest(QUrl(QString("http://www.cinefacts.de%1").arg(posters.takeFirst()))));
            reply->setProperty("storage", Storage::toVariant(reply, movie));
            reply->setProperty("posters", posters);
            reply->setProperty("backdrops", backdrops);
            reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
            connect(reply, SIGNAL(finished()), this, SLOT(posterFinished()));
            return;
        }

        if (!backdrops.isEmpty() && reply->property("infosToLoad").value<Storage*>()->infosToLoad().contains(MovieScraperInfos::Backdrop)) {
            reply = qnam()->get(QNetworkRequest(QUrl(QString("http://www.cinefacts.de%1").arg(backdrops.takeFirst()))));
            reply->setProperty("storage", Storage::toVariant(reply, movie));
            reply->setProperty("backdrops", backdrops);
            reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
            connect(reply, SIGNAL(finished()), this, SLOT(backdropFinished()));
            return;
        }
    }
    movie->controller()->scraperLoadDone(this);
}

/**
 * @brief Called when backdrop scraping has finished
 * Starts the next backdrop download or tells the movie that scraping is done
 */
void Cinefacts::backdropFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    QStringList backdrops = reply->property("backdrops").toStringList();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    if (!movie)
        return;

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        QRegExp rx("<a href=\"([^\"]*)\" target=\"_blank\">Bild in Originalgr..e</a>");
        rx.setMinimal(true);
        if (rx.indexIn(msg) != -1) {
            Poster p;
            p.thumbUrl = rx.cap(1);
            p.originalUrl = rx.cap(1);
            movie->addBackdrop(p);
        }

        if (!backdrops.isEmpty()) {
            reply = qnam()->get(QNetworkRequest(QUrl(QString("http://www.cinefacts.de%1").arg(backdrops.takeFirst()))));
            reply->setProperty("storage", Storage::toVariant(reply, movie));
            reply->setProperty("backdrops", backdrops);
            reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
            connect(reply, SIGNAL(finished()), this, SLOT(backdropFinished()));
            return;
        }
    }
    movie->controller()->scraperLoadDone(this);
}
