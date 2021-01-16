#include "scrapers/movie/ofdb/OFDb.h"

#include "data/Storage.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "network/NetworkRequest.h"
#include "scrapers/movie/ofdb/OfdbSearchJob.h"
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

MovieSearchJob* OFDb::search(MovieSearchJob::Config config)
{
    return new OfdbSearchJob(m_api, std::move(config), this);
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
 * \brief Starts network requests to download infos from OFDb
 * \param ids OFDb movie ID
 * \param movie Movie object
 * \param infos List of infos to load
 * \see OFDb::loadFinished
 */
void OFDb::loadData(QHash<MovieScraper*, mediaelch::scraper::MovieIdentifier> ids,
    Movie* movie,
    QSet<MovieScraperInfo> infos)
{
    movie->clear(infos);

    QUrl url(QStringLiteral("https://ofdbgw.geeksphere.de/movie/%1").arg(ids.values().first().str()));
    auto request = mediaelch::network::requestWithDefaults(url);
    QNetworkReply* reply = network()->getWithWatcher(request);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("ofdbId", ids.values().first().str());
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

    auto dls = makeDeleteLaterScope(reply);

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

    ScraperError error;

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);
    } else {
        error = mediaelch::replyToScraperError(*reply);
    }

    movie->controller()->scraperLoadDone(this, error);
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
        if (xml.name() != QLatin1String("resultat")) {
            xml.skipCurrentElement();
        } else {
            break;
        }
    }

    while (xml.readNextStartElement()) {
        if (infos.contains(MovieScraperInfo::Title) && xml.name() == QLatin1String("titel")) {
            movie->setName(xml.readElementText());
        } else if (infos.contains(MovieScraperInfo::Released) && xml.name() == QLatin1String("jahr")) {
            movie->setReleased(QDate::fromString(xml.readElementText(), "yyyy"));
        } else if (infos.contains(MovieScraperInfo::Poster) && xml.name() == QLatin1String("bild")) {
            QString url = xml.readElementText();
            Poster p;
            p.originalUrl = QUrl(url);
            p.thumbUrl = QUrl(url);
            movie->images().addPoster(p);
        } else if (infos.contains(MovieScraperInfo::Rating) && xml.name() == QLatin1String("bewertung")) {
            while (xml.readNextStartElement()) {
                if (xml.name() == QLatin1String("note")) {
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
        } else if (infos.contains(MovieScraperInfo::Genres) && xml.name() == QLatin1String("genre")) {
            while (xml.readNextStartElement()) {
                if (xml.name() == QLatin1String("titel")) {
                    movie->addGenre(helper::mapGenre(xml.readElementText()));
                } else {
                    xml.skipCurrentElement();
                }
            }
        } else if (infos.contains(MovieScraperInfo::Actors) && xml.name() == QLatin1String("besetzung")) {
            // clear actors
            movie->setActors({});

            while (xml.readNextStartElement()) {
                if (xml.name() != QLatin1String("person")) {
                    xml.skipCurrentElement();
                } else {
                    Actor actor;
                    while (xml.readNextStartElement()) {
                        if (xml.name() == QLatin1String("name")) {
                            actor.name = xml.readElementText();
                        } else if (xml.name() == QLatin1String("rolle")) {
                            actor.role = xml.readElementText();
                        } else {
                            xml.skipCurrentElement();
                        }
                    }
                    movie->addActor(actor);
                }
            }
        } else if (infos.contains(MovieScraperInfo::Countries) && xml.name() == QLatin1String("produktionsland")) {
            while (xml.readNextStartElement()) {
                if (xml.name() == QLatin1String("name")) {
                    movie->addCountry(helper::mapCountry(xml.readElementText()));
                } else {
                    xml.skipCurrentElement();
                }
            }
        } else if (infos.contains(MovieScraperInfo::Title) && xml.name() == QLatin1String("alternativ")) {
            movie->setOriginalName(xml.readElementText());
        } else if (infos.contains(MovieScraperInfo::Overview) && xml.name() == QLatin1String("beschreibung")) {
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
