#include "scrapers/movie/ofdb/OFDb.h"

#include "data/Storage.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "network/NetworkRequest.h"
#include "settings/Settings.h"

#include <QDomDocument>
#include <QRegularExpression>
#include <QWidget>
#include <QXmlStreamReader>

namespace mediaelch {
namespace scraper {

/// \brief OFDb scraper. Uses http://ofdbgw.metawave.ch directly because ttp://www.ofdbgw.org
/// is as of 2019-02-23 down.
OFDb::OFDb(QObject* parent) : MovieScraper(parent)
{
    m_meta.identifier = ID;
    m_meta.name = "OFDb";
    m_meta.description = tr("OFDb is a German online movie database.");
    m_meta.website = "https://ssl.ofdb.de/";
    m_meta.termsOfService = "https://ssl.ofdb.de/view.php?page=info#rechtliches";
    m_meta.privacyPolicy = "https://ssl.ofdb.de/view.php?page=info#datenschutz";
    m_meta.help = "https://www.gemeinschaftsforum.com/forum/";
    m_meta.supportedDetails = {MovieScraperInfo::Title,
        MovieScraperInfo::Released,
        MovieScraperInfo::Poster,
        MovieScraperInfo::Rating,
        MovieScraperInfo::Genres,
        MovieScraperInfo::Actors,
        MovieScraperInfo::Countries,
        MovieScraperInfo::Overview};
    m_meta.supportedLanguages = {"de"};
    m_meta.defaultLocale = "de";
    m_meta.isAdult = false;
}

const MovieScraper::ScraperMeta& OFDb::meta() const
{
    return m_meta;
}

void OFDb::initialize()
{
    // no-op
    // OFDb requires no initialization.
}

bool OFDb::isInitialized() const
{
    // OFDb requires no initialization.
    return true;
}

bool OFDb::hasSettings() const
{
    return false;
}

void OFDb::loadSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

void OFDb::saveSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

/**
 * \brief Just returns a pointer to the scrapers network access manager
 * \return Network Access Manager
 */
mediaelch::network::NetworkManager* OFDb::network()
{
    return &m_network;
}

QSet<MovieScraperInfo> OFDb::scraperNativelySupports()
{
    return m_meta.supportedDetails;
}

void OFDb::changeLanguage(mediaelch::Locale /*locale*/)
{
    // no-op: Only one language is supported and it is hard-coded.
}

/**
 * \brief Searches for a movie
 * \param searchStr The Movie name/search string
 * \see OFDb::searchFinished
 */
void OFDb::search(QString searchStr)
{
    qDebug() << "Entered, searchStr=" << searchStr;

    QString encodedSearch = helper::toLatin1PercentEncoding(searchStr);

    QUrl url;
    QRegularExpression rx("^id\\d+$"); // special handling if search string is an ID
    if (rx.match(searchStr).hasMatch()) {
        url.setUrl(QStringLiteral("http://ofdbgw.metawave.ch/movie/%1").arg(searchStr.mid(2)).toUtf8());
    } else {
        url.setUrl(QStringLiteral("http://ofdbgw.metawave.ch/search/%1").arg(encodedSearch).toUtf8());
    }
    auto request = mediaelch::network::requestWithDefaults(url);
    QNetworkReply* reply = network()->getWithWatcher(request);
    reply->setProperty("searchString", searchStr);
    reply->setProperty("notFoundCounter", 0);
    connect(reply, &QNetworkReply::finished, this, &OFDb::searchFinished);
}

/**
 * \brief Called when the search result was downloaded
 *        Emits "searchDone" if there are no more pages in the result set
 * \see OFDb::parseSearch
 */
void OFDb::searchFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply == nullptr) {
        qCritical() << "[OFDb] onSearchFinished: nullptr reply | Please report this issue!";
        emit searchDone(
            {}, {ScraperError::Type::InternalError, tr("Internal Error: Please report!"), "nullptr dereference"});
        return;
    }
    reply->deleteLater();

    QString searchStr = reply->property("searchString").toString();
    int notFoundCounter = reply->property("notFoundCounter").toInt();

    // try to get another mirror when 404 occurs
    if (reply->error() == QNetworkReply::ContentNotFoundError) {
        qWarning() << "Got 404";
        if (notFoundCounter < 3) {
            ++notFoundCounter;
            reply->deleteLater();
            // New request.
            QUrl url(QString("http://ofdbgw.geeksphere.de/search/%1").arg(searchStr));
            auto request = mediaelch::network::requestWithDefaults(url);
            reply = network()->get(request);
            reply->setProperty("searchString", searchStr);
            reply->setProperty("notFoundCounter", notFoundCounter);
            connect(reply, &QNetworkReply::finished, this, &OFDb::searchFinished);
            return;
        }
        qWarning() << "[OFDb] Too many 404 errors. Quit search.";
        emit searchDone(
            {}, {ScraperError::Type::NetworkError, tr("Too many redirects, can't load search results!"), {}});
    }


    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[OFDb] Search: Network Error" << reply->errorString();
        emit searchDone({}, mediaelch::replyToScraperError(*reply));
        return;
    }

    const QString msg = QString::fromUtf8(reply->readAll());
    auto results = parseSearch(msg, searchStr);
    emit searchDone(results, {});
}

/**
 * \brief Parses the search results
 * \param xml XML data
 * \return List of search results
 */
QVector<ScraperSearchResult> OFDb::parseSearch(QString xml, QString searchStr)
{
    QVector<ScraperSearchResult> results;
    QDomDocument domDoc;
    domDoc.setContent(xml);

    if (domDoc.elementsByTagName("eintrag").count() == 0 && !domDoc.elementsByTagName("resultat").isEmpty()) {
        QDomElement entry = domDoc.elementsByTagName("resultat").at(0).toElement();
        ScraperSearchResult result;
        result.id = searchStr.mid(2);
        if (entry.elementsByTagName("titel").size() > 0) {
            result.name = entry.elementsByTagName("titel").at(0).toElement().text();
        }
        if (entry.elementsByTagName("jahr").size() > 0) {
            result.released = QDate::fromString(entry.elementsByTagName("jahr").at(0).toElement().text(), "yyyy");
        }
        results.append(result);
    } else {
        for (int i = 0, n = domDoc.elementsByTagName("eintrag").size(); i < n; i++) {
            QDomElement entry = domDoc.elementsByTagName("eintrag").at(i).toElement();
            if (entry.elementsByTagName("id").size() == 0
                || entry.elementsByTagName("id").at(0).toElement().text().isEmpty()) {
                continue;
            }
            ScraperSearchResult result;
            result.id = entry.elementsByTagName("id").at(0).toElement().text();
            if (entry.elementsByTagName("titel").size() > 0) {
                result.name = entry.elementsByTagName("titel").at(0).toElement().text();
            }
            if (entry.elementsByTagName("jahr").size() > 0) {
                result.released = QDate::fromString(entry.elementsByTagName("jahr").at(0).toElement().text(), "yyyy");
            }
            results.append(result);
        }
    }
    return results;
}

/**
 * \brief Starts network requests to download infos from OFDb
 * \param ids OFDb movie ID
 * \param movie Movie object
 * \param infos List of infos to load
 * \see OFDb::loadFinished
 */
void OFDb::loadData(QHash<MovieScraper*, QString> ids, Movie* movie, QSet<MovieScraperInfo> infos)
{
    movie->clear(infos);

    QUrl url(QStringLiteral("http://ofdbgw.geeksphere.de/movie/%1").arg(ids.values().first()));
    auto request = mediaelch::network::requestWithDefaults(url);
    QNetworkReply* reply = network()->getWithWatcher(request);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("ofdbId", ids.values().first());
    reply->setProperty("notFoundCounter", 0);
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, &QNetworkReply::finished, this, &OFDb::loadFinished);
}

/**
 * \brief Called when the movie infos are downloaded
 * \see OFDb::parseAndAssignInfos
 */
void OFDb::loadFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Movie* movie = reply->property("storage").value<Storage*>()->movie();
    QString ofdbId = reply->property("ofdbId").toString();
    QSet<MovieScraperInfo> infos = reply->property("infosToLoad").value<Storage*>()->movieInfosToLoad();
    int notFoundCounter = reply->property("notFoundCounter").toInt();
    if (movie == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::ContentNotFoundError && notFoundCounter < 3) {
        qWarning() << "Got 404";
        notFoundCounter++;
        reply->deleteLater();
        QUrl url(QString("http://ofdbgw.metawave.ch/movie/%1").arg(ofdbId));
        auto request = mediaelch::network::requestWithDefaults(url);
        reply = network()->get(request);
        reply->setProperty("storage", Storage::toVariant(reply, movie));
        reply->setProperty("ofdbId", ofdbId);
        reply->setProperty("notFoundCounter", notFoundCounter);
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, &QNetworkReply::finished, this, &OFDb::loadFinished);
        return;
    }


    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);
    } else {
        showNetworkError(*reply);
        qWarning() << "Network Error" << reply->errorString();
    }
    reply->deleteLater();
    movie->controller()->scraperLoadDone(this);
}

/**
 * \brief Parses HTML data and assigns it to the given movie object
 * \param data HTML data
 * \param movie Movie object
 * \param infos List of infos to load
 */
void OFDb::parseAndAssignInfos(QString data, Movie* movie, QSet<MovieScraperInfo> infos)
{
    QXmlStreamReader xml(data);

    if (!xml.readNextStartElement()) {
        qWarning() << "[OFDb] XML has unexpected structure; couldn't read root element";
        return;
    }

    while (xml.readNextStartElement()) {
        if (xml.name() != "resultat") {
            xml.skipCurrentElement();
        } else {
            break;
        }
    }

    while (xml.readNextStartElement()) {
        if (infos.contains(MovieScraperInfo::Title) && xml.name() == "titel") {
            movie->setName(xml.readElementText());
        } else if (infos.contains(MovieScraperInfo::Released) && xml.name() == "jahr") {
            movie->setReleased(QDate::fromString(xml.readElementText(), "yyyy"));
        } else if (infos.contains(MovieScraperInfo::Poster) && xml.name() == "bild") {
            QString url = xml.readElementText();
            Poster p;
            p.originalUrl = QUrl(url);
            p.thumbUrl = QUrl(url);
            movie->images().addPoster(p);
        } else if (infos.contains(MovieScraperInfo::Rating) && xml.name() == "bewertung") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "note") {
                    Rating rating;
                    rating.source = "OFDb";
                    rating.rating = xml.readElementText().toDouble();
                    if (movie->ratings().isEmpty()) {
                        movie->ratings().push_back(rating);
                    } else {
                        movie->ratings().back() = rating;
                    }

                } else {
                    xml.skipCurrentElement();
                }
            }
        } else if (infos.contains(MovieScraperInfo::Genres) && xml.name() == "genre") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "titel") {
                    movie->addGenre(helper::mapGenre(xml.readElementText()));
                } else {
                    xml.skipCurrentElement();
                }
            }
        } else if (infos.contains(MovieScraperInfo::Actors) && xml.name() == "besetzung") {
            // clear actors
            movie->setActors({});

            while (xml.readNextStartElement()) {
                if (xml.name() != "person") {
                    xml.skipCurrentElement();
                } else {
                    Actor actor;
                    while (xml.readNextStartElement()) {
                        if (xml.name() == "name") {
                            actor.name = xml.readElementText();
                        } else if (xml.name() == "rolle") {
                            actor.role = xml.readElementText();
                        } else {
                            xml.skipCurrentElement();
                        }
                    }
                    movie->addActor(actor);
                }
            }
        } else if (infos.contains(MovieScraperInfo::Countries) && xml.name() == "produktionsland") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "name") {
                    movie->addCountry(helper::mapCountry(xml.readElementText()));
                } else {
                    xml.skipCurrentElement();
                }
            }
        } else if (infos.contains(MovieScraperInfo::Title) && xml.name() == "alternativ") {
            movie->setOriginalName(xml.readElementText());
        } else if (infos.contains(MovieScraperInfo::Overview) && xml.name() == "beschreibung") {
            movie->setOverview(xml.readElementText());
            if (Settings::instance()->usePlotForOutline()) {
                movie->setOutline(xml.readElementText());
            }
        } else {
            xml.skipCurrentElement();
        }
    }
}

QWidget* OFDb::settingsWidget()
{
    return nullptr;
}

} // namespace scraper
} // namespace mediaelch
