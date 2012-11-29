#include "OFDb.h"

#include <QDomDocument>
#include <QXmlStreamReader>
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "settings/Settings.h"

/**
 * @brief OFDb::OFDb
 * @param parent
 */
OFDb::OFDb(QObject *parent)
{
    setParent(parent);
    m_httpNotFoundCounter = 0;
    m_scraperSupports << MovieScraperInfos::Title
                      << MovieScraperInfos::Released
                      << MovieScraperInfos::Poster
                      << MovieScraperInfos::Rating
                      << MovieScraperInfos::Genres
                      << MovieScraperInfos::Actors
                      << MovieScraperInfos::Countries
                      << MovieScraperInfos::Overview;
}

/**
 * @brief Returns the name of the scraper
 * @return Name of the Scraper
 */
QString OFDb::name()
{
    return QString("OFDb");
}

/**
 * @brief Returns if the scraper has settings
 * @return Scraper has settings
 */
bool OFDb::hasSettings()
{
    return false;
}

/**
 * @brief Loads scrapers settings
 */
void OFDb::loadSettings()
{
}

/**
 * @brief Saves scrapers settings
 */
void OFDb::saveSettings()
{
}

/**
 * @brief Just returns a pointer to the scrapers network access manager
 * @return Network Access Manager
 */
QNetworkAccessManager *OFDb::qnam()
{
    return &m_qnam;
}

/**
 * @brief Returns a list of infos available from the scraper
 * @return List of supported infos
 */
QList<int> OFDb::scraperSupports()
{
    return m_scraperSupports;
}

/**
 * @brief Searches for a movie
 * @param searchStr The Movie name/search string
 * @see OFDb::searchFinished
 */
void OFDb::search(QString searchStr)
{
    qDebug() << "Entered, searchStr=" << searchStr;
    m_httpNotFoundCounter = 0;
    m_currentSearchString = searchStr;

    QString encodedSearch = Helper::toLatin1PercentEncoding(searchStr);
    QUrl url;
    url.setEncodedUrl(QString("http://www.ofdbgw.org/search/%1").arg(encodedSearch).toUtf8());
    m_searchReply = this->qnam()->get(QNetworkRequest(url));
    connect(m_searchReply, SIGNAL(finished()), this, SLOT(searchFinished()));
}

/**
 * @brief Called when the search result was downloaded
 *        Emits "searchDone" if there are no more pages in the result set
 * @see OFDb::parseSearch
 */
void OFDb::searchFinished()
{
    qDebug() << "Entered";
    if (m_searchReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 302 ||
        m_searchReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 301) {
        qDebug() << "Got redirect" << m_searchReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        m_searchReply->deleteLater();
        m_searchReply = this->qnam()->get(QNetworkRequest(m_searchReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl()));
        connect(m_searchReply, SIGNAL(finished()), this, SLOT(searchFinished()));
        return;
    }

    // try to get another mirror when 404 occurs
    if (m_searchReply->error() == QNetworkReply::ContentNotFoundError && m_httpNotFoundCounter < 3) {
        qWarning() << "Got 404";
        m_httpNotFoundCounter++;
        m_searchReply->deleteLater();
        QUrl url(QString("http://www.ofdbgw.org/search/%1").arg(m_currentSearchString));
        m_searchReply = this->qnam()->get(QNetworkRequest(url));
        connect(m_searchReply, SIGNAL(finished()), this, SLOT(searchFinished()));
        return;
    }

    QList<ScraperSearchResult> results;
    if (m_searchReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_searchReply->readAll());
        results = parseSearch(msg);
    } else {
        qWarning() << "Network Error" << m_searchReply->errorString();
    }
    m_searchReply->deleteLater();
    emit searchDone(results);
}

/**
 * @brief Parses the search results
 * @param xml XML data
 * @return List of search results
 */
QList<ScraperSearchResult> OFDb::parseSearch(QString xml)
{
    qDebug() << "Entered";
    QList<ScraperSearchResult> results;
    QDomDocument domDoc;
    domDoc.setContent(xml);
    for (int i=0, n=domDoc.elementsByTagName("eintrag").size() ; i<n ; i++) {
        QDomElement entry = domDoc.elementsByTagName("eintrag").at(i).toElement();
        if (entry.elementsByTagName("id").size() == 0 || entry.elementsByTagName("id").at(0).toElement().text().isEmpty())
            continue;
        ScraperSearchResult result;
        result.id = entry.elementsByTagName("id").at(0).toElement().text();
        if (entry.elementsByTagName("titel").size() > 0)
            result.name = entry.elementsByTagName("titel").at(0).toElement().text();
        if (entry.elementsByTagName("jahr").size() > 0)
            result.released = QDate::fromString(entry.elementsByTagName("jahr").at(0).toElement().text(), "yyyy");
        results.append(result);
    }
    return results;
}

/**
 * @brief Starts network requests to download infos from OFDb
 * @param id OFDb movie ID
 * @param movie Movie object
 * @param infos List of infos to load
 * @see OFDb::loadFinished
 */
void OFDb::loadData(QString id, Movie *movie, QList<int> infos)
{
    qDebug() << "Entered, id=" << id << "movie=" << movie->name();
    m_infosToLoad = infos;
    m_httpNotFoundCounter = 0;
    m_currentLoadId = id;
    m_currentMovie = movie;
    QUrl url(QString("http://ofdbgw.org/movie/%1").arg(id));
    m_loadReply = this->qnam()->get(QNetworkRequest(url));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(loadFinished()));
}

/**
 * @brief Called when the movie infos are downloaded
 * @see OFDb::parseAndAssignInfos
 */
void OFDb::loadFinished()
{
    qDebug() << "Entered";
    if (m_loadReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 302 ||
        m_loadReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 301) {
        qDebug() << "Got redirect" << m_loadReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        m_loadReply->deleteLater();
        m_loadReply = this->qnam()->get(QNetworkRequest(m_loadReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl()));
        connect(m_loadReply, SIGNAL(finished()), this, SLOT(loadFinished()));
        return;
    }

    if (m_loadReply->error() == QNetworkReply::ContentNotFoundError && m_httpNotFoundCounter < 3) {
        qWarning() << "Got 404";
        m_httpNotFoundCounter++;
        m_loadReply->deleteLater();
        QUrl url(QString("http://ofdbgw.org/movie/%1").arg(m_currentLoadId));
        m_loadReply = this->qnam()->get(QNetworkRequest(url));
        connect(m_loadReply, SIGNAL(finished()), this, SLOT(loadFinished()));
        return;
    }


    if (m_loadReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_loadReply->readAll());
        parseAndAssignInfos(msg, m_currentMovie, m_infosToLoad);
    } else {
        qWarning() << "Network Error" << m_loadReply->errorString();
    }
    m_loadReply->deleteLater();
    m_currentMovie->scraperLoadDone();
}

/**
 * @brief Parses HTML data and assigns it to the given movie object
 * @param data HTML data
 * @param movie Movie object
 * @param infos List of infos to load
 */
void OFDb::parseAndAssignInfos(QString data, Movie *movie, QList<int> infos)
{
    qDebug() << "Entered";
    movie->clear(infos);
    QXmlStreamReader xml(data);

    xml.readNextStartElement();
    while (xml.readNextStartElement()) {
        if (xml.name() != "resultat")
            xml.skipCurrentElement();
        else
            break;
    }

    while (xml.readNextStartElement()) {
        if (infos.contains(MovieScraperInfos::Title) && xml.name() == "titel") {
            movie->setName(xml.readElementText());
        } else if (infos.contains(MovieScraperInfos::Released) && xml.name() == "jahr") {
            movie->setReleased(QDate::fromString(xml.readElementText(), "yyyy"));
        } else if (infos.contains(MovieScraperInfos::Poster) && xml.name() == "bild") {
            QString url = xml.readElementText();
            Poster p;
            p.originalUrl = QUrl(url);
            p.thumbUrl = QUrl(url);
            movie->addPoster(p);
        } else if (infos.contains(MovieScraperInfos::Rating) && xml.name() == "bewertung") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "note")
                    movie->setRating(xml.readElementText().toFloat());
                else
                    xml.skipCurrentElement();
            }
        } else if (infos.contains(MovieScraperInfos::Genres) && xml.name() == "genre") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "titel")
                    movie->addGenre(xml.readElementText());
                else
                    xml.skipCurrentElement();
            }
        } else if (infos.contains(MovieScraperInfos::Actors) && xml.name() == "besetzung") {
            while (xml.readNextStartElement()) {
                if (xml.name() != "person") {
                    xml.skipCurrentElement();
                } else {
                    Actor actor;
                    while (xml.readNextStartElement()) {
                        if (xml.name() == "name")
                            actor.name = xml.readElementText();
                        else if (xml.name() == "rolle")
                            actor.role = xml.readElementText();
                        else
                            xml.skipCurrentElement();
                    }
                    movie->addActor(actor);
                }
            }
        } else if (infos.contains(MovieScraperInfos::Countries) && xml.name() == "produktionsland") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "name")
                    movie->addCountry(xml.readElementText());
                else
                    xml.skipCurrentElement();
            }
        } else if (infos.contains(MovieScraperInfos::Title) && xml.name() == "alternativ") {
            movie->setOriginalName(xml.readElementText());
        } else if (infos.contains(MovieScraperInfos::Overview) && xml.name() == "beschreibung") {
            movie->setOverview(xml.readElementText());
            if (Settings::instance()->usePlotForOutline())
                movie->setOutline(xml.readElementText());
        } else {
            xml.skipCurrentElement();
        }
    }
}

/**
 * @brief OFDb::languages
 * @return
 */
QMap<QString, QString> OFDb::languages()
{
    QMap<QString, QString> m;
    return m;
}

/**
 * @brief language
 * @return
 */
QString OFDb::language()
{
    return QString();
}

/**
 * @brief OFDb::setLanguage
 * @param language
 */
void OFDb::setLanguage(QString language)
{
    Q_UNUSED(language);
}
