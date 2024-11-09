#include "ConcertXmlReader.h"

#include "data/concert/Concert.h"
#include "media/StreamDetails.h"
#include "media_center/KodiXml.h"

#include <QDate>
#include <QDomDocument>
#include <QDomNodeList>
#include <QStringList>
#include <QUrl>
#include <array>

namespace mediaelch {
namespace kodi {

ConcertXmlReader::ConcertXmlReader(Concert& concert) : m_concert{concert}
{
}

void ConcertXmlReader::parse(QXmlStreamReader& reader)
{
    if (reader.readNextStartElement()) {
        // There is only "musicvideo" for concerts. But in case that there are other
        // NFO files, we also accept other root element names.
        if (reader.name() == QLatin1String("musicvideo") || reader.name() == QLatin1String("concert")
            || reader.name() == QLatin1String("movie")) {
            parseConcert(reader);
        } else {
            reader.raiseError(QObject::tr("No valid musicvideo root entry found"));
        }
    }
}

void ConcertXmlReader::parseConcert(QXmlStreamReader& reader)
{
    Rating oldStyleRating;
    QStringList artists;
    while (reader.readNextStartElement()) {
        if (reader.name() == QLatin1String("id")) {
            // v16 imdbid
            m_concert.setImdbId(ImdbId(reader.readElementText()));

        } else if (reader.name() == QLatin1String("tmdbid")) {
            // v16 tmdbid
            m_concert.setTmdbId(TmdbId(reader.readElementText()));

        } else if (reader.name() == QLatin1String("title")) {
            m_concert.setTitle(reader.readElementText());

        } else if (reader.name() == QLatin1String("originaltitle")) {
            m_concert.setOriginalTitle(reader.readElementText());

        } else if (reader.name() == QLatin1String("artist")) {
            artists << reader.readElementText();

        } else if (reader.name() == QLatin1String("album")) {
            m_concert.setAlbum(reader.readElementText());

        } else if (reader.name() == QLatin1String("userrating")) {
            m_concert.setUserRating(reader.readElementText().toDouble());

        } else if (reader.name() == QLatin1String("year")) {
            m_concert.setReleased(QDate::fromString(reader.readElementText(), "yyyy"));

        } else if (reader.name() == QLatin1String("plot")) {
            m_concert.setOverview(reader.readElementText());

        } else if (reader.name() == QLatin1String("tagline")) {
            m_concert.setTagline(reader.readElementText());

        } else if (reader.name() == QLatin1String("runtime")) {
            m_concert.setRuntime(std::chrono::minutes(reader.readElementText().toInt()));

        } else if (reader.name() == QLatin1String("mpaa")) {
            m_concert.setCertification(Certification(reader.readElementText()));

        } else if (reader.name() == QLatin1String("playcount")) {
            m_concert.setPlayCount(reader.readElementText().toInt());

        } else if (reader.name() == QLatin1String("lastplayed")) {
            m_concert.setLastPlayed(QDateTime::fromString(reader.readElementText(), "yyyy-MM-dd HH:mm:ss"));

        } else if (reader.name() == QLatin1String("trailer")) {
            m_concert.setTrailer(QUrl(reader.readElementText()));

        } else if (reader.name() == QLatin1String("genre")) {
            const QStringList genres = reader.readElementText().split(" / ", ElchSplitBehavior::SkipEmptyParts);
            for (const QString& genre : genres) {
                m_concert.addGenre(genre);
            }

        } else if (reader.name() == QLatin1String("tag")) {
            m_concert.addTag(reader.readElementText());


        } else if (reader.name() == QLatin1String("uniqueid")) {
            parseUniqueId(reader);

        } else if (reader.name() == QLatin1String("ratings")) {
            parseRatings(reader);

        } else if (reader.name() == QLatin1String("rating")) {
            // otherwise use "old" syntax:
            // <rating>10.0</rating>
            // <votes>10.0</votes>
            QString value = reader.readElementText();
            if (!value.isEmpty()) {
                oldStyleRating.rating = value.replace(",", ".").toDouble();
            }

        } else if (reader.name() == QLatin1String("votes")) {
            QString value = reader.readElementText();
            if (!value.isEmpty()) {
                oldStyleRating.voteCount = value.replace(",", "").replace(".", "").toInt();
            }

        } else if (reader.name() == QLatin1String("thumb")) {
            parsePoster(reader);

        } else if (reader.name() == QLatin1String("fanart")) {
            parseFanart(reader);

        } else if (reader.name() == QLatin1String("fileinfo")) {
            while (reader.readNextStartElement()) {
                if (reader.name() == QLatin1String("streamdetails")) {
                    KodiXml::parseStreamDetails(reader, m_concert.streamDetails());

                } else {
                    reader.skipCurrentElement();
                }
            }
        } else {
            reader.skipCurrentElement();
        }
    }

    m_concert.setArtists(artists);

    if (oldStyleRating.rating > 0) {
        m_concert.ratings().setOrAddRating(oldStyleRating);
        m_concert.setChanged(true);
    }
}

void mediaelch::kodi::ConcertXmlReader::parseUniqueId(QXmlStreamReader& reader)
{
    QString type = reader.attributes().value("type").toString();
    QString value = reader.readElementText();

    if (type == "imdb") {
        m_concert.setImdbId(ImdbId(value));
    } else if (type == "tmdb") {
        m_concert.setTmdbId(TmdbId(value));
    }
}

void ConcertXmlReader::parseRatings(QXmlStreamReader& reader)
{
    m_concert.ratings().clear();
    // <ratings>
    //   <rating name="default" default="true" min="0" max="10">
    //     <value>10</value>
    //     <votes>10</votes>
    //   </rating>
    // </ratings>
    while (reader.readNextStartElement()) {
        if (reader.name() == QLatin1String("rating")) {
            Rating rating;
            rating.source = reader.attributes().value("name").toString();
            if (rating.source.isEmpty()) {
                rating.source = "default";
            }

            if (reader.attributes().hasAttribute("max")) {
                const QString maxStr = reader.attributes().value("max").toString();
                if (!maxStr.isEmpty()) {
                    bool ok = false;
                    const int max = maxStr.toInt(&ok);
                    if (ok && max > 0) {
                        rating.maxRating = max;
                    }
                }
            }

            if (reader.attributes().hasAttribute("min")) {
                const QString minStr = reader.attributes().value("min").toString();
                if (!minStr.isEmpty()) {
                    bool ok = false;
                    const int min = minStr.toInt(&ok);
                    if (ok && min >= 0) {
                        rating.minRating = min;
                    }
                }
            }

            while (reader.readNextStartElement()) {
                if (reader.name() == QLatin1String("value")) {
                    rating.rating = reader.readElementText().replace(",", ".").toDouble();

                } else if (reader.name() == QLatin1String("votes")) {
                    rating.voteCount = reader.readElementText().replace(",", "").replace(".", "").toInt();

                } else {
                    reader.skipCurrentElement();
                }
            }

            m_concert.ratings().setOrAddRating(rating);
            m_concert.setChanged(true);
        } else {
            reader.skipCurrentElement();
        }
    }
}

void ConcertXmlReader::parseFanart(QXmlStreamReader& reader)
{
    while (reader.readNextStartElement()) {
        if (reader.name() != QLatin1String("thumb")) {
            reader.skipCurrentElement();
            continue;
        }

        Poster p;
        p.thumbUrl = reader.attributes().value("preview").toString();
        p.aspect = reader.attributes().value("aspect").toString();
        p.originalUrl = reader.readElementText();

        if (!p.thumbUrl.isValid()) {
            p.thumbUrl = p.originalUrl;
        }

        if (p.originalUrl.isValid()) {
            m_concert.addBackdrop(p);
        }
    }
}

void ConcertXmlReader::parsePoster(QXmlStreamReader& reader)
{
    Poster p;
    p.thumbUrl = reader.attributes().value("preview").toString();
    p.aspect = reader.attributes().value("aspect").toString();
    p.originalUrl = reader.readElementText();

    if (!p.thumbUrl.isValid()) {
        p.thumbUrl = p.originalUrl;
    }

    if (p.originalUrl.isValid()) {
        m_concert.addPoster(p);
    }
}


} // namespace kodi
} // namespace mediaelch
