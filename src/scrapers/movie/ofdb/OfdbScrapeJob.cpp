#include "scrapers/movie/ofdb/OfdbScrapeJob.h"

#include "globals/Helper.h"
#include "movies/Movie.h"
#include "scrapers/movie/ofdb/OfdbApi.h"

#include <QXmlStreamReader>

namespace mediaelch {
namespace scraper {

OfdbScrapeJob::OfdbScrapeJob(OfdbApi& api, MovieScrapeJob::Config _config, QObject* parent) :
    MovieScrapeJob(std::move(_config), parent), m_api{api}
{
}

void OfdbScrapeJob::execute()
{
    m_api.loadMovie(config().identifier.str(), [this](QString html, ScraperError error) {
        if (!error.hasError()) {
            parseAndAssignInfos(html);
        } else {
            m_error = error;
        }

        emit sigFinished(this);
    });
}

void OfdbScrapeJob::parseAndAssignInfos(const QString& data)
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
        if (xml.name() == "titel") {
            m_movie->setName(xml.readElementText());

        } else if (xml.name() == "jahr") {
            m_movie->setReleased(QDate::fromString(xml.readElementText(), "yyyy"));

        } else if (xml.name() == "bild") {
            QString url = xml.readElementText();
            Poster p;
            p.originalUrl = QUrl(url);
            p.thumbUrl = QUrl(url);
            m_movie->images().addPoster(p);

        } else if (xml.name() == "bewertung") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "note") {
                    Rating rating;
                    rating.source = "OFDb";
                    rating.rating = xml.readElementText().toDouble();
                    if (m_movie->ratings().isEmpty()) {
                        m_movie->ratings().push_back(rating);
                    } else {
                        m_movie->ratings().back() = rating;
                    }

                } else {
                    xml.skipCurrentElement();
                }
            }
        } else if (xml.name() == "genre") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "titel") {
                    m_movie->addGenre(helper::mapGenre(xml.readElementText()));
                } else {
                    xml.skipCurrentElement();
                }
            }

        } else if (xml.name() == "besetzung") {
            // clear actors
            m_movie->setActors({});

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
                    m_movie->addActor(actor);
                }
            }

        } else if (xml.name() == "produktionsland") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "name") {
                    m_movie->addCountry(helper::mapCountry(xml.readElementText()));
                } else {
                    xml.skipCurrentElement();
                }
            }

        } else if (xml.name() == "alternativ") {
            m_movie->setOriginalName(xml.readElementText());

        } else if (xml.name() == "beschreibung") {
            m_movie->setOverview(xml.readElementText());

        } else {
            xml.skipCurrentElement();
        }
    }
}
} // namespace scraper
} // namespace mediaelch
