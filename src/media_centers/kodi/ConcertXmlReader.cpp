#include "ConcertXmlReader.h"

#include "concerts/Concert.h"
#include "data/StreamDetails.h"

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
        if (reader.name() == "musicvideo" || reader.name() == "concert" || reader.name() == "movie") {
            parseConcert(reader);
        } else {
            reader.raiseError(QObject::tr("Not a cookbook file"));
        }
    }
}

void ConcertXmlReader::parseConcert(QXmlStreamReader& reader)
{
    Rating oldStyleRating;
    while (reader.readNextStartElement()) {
        if (reader.name() == "id") {
            // v16 imdbid
            m_concert.setImdbId(ImdbId(reader.readElementText()));

        } else if (reader.name() == "tmdbid") {
            // v16 tmdbid
            m_concert.setTmdbId(TmdbId(reader.readElementText()));

        } else if (reader.name() == "title") {
            m_concert.setTitle(reader.readElementText());

        } else if (reader.name() == "originaltitle") {
            m_concert.setOriginalTitle(reader.readElementText());

        } else if (reader.name() == "artist") {
            m_concert.setArtist(reader.readElementText());

        } else if (reader.name() == "album") {
            m_concert.setAlbum(reader.readElementText());

        } else if (reader.name() == "userrating") {
            m_concert.setUserRating(reader.readElementText().toDouble());

        } else if (reader.name() == "year") {
            m_concert.setReleased(QDate::fromString(reader.readElementText(), "yyyy"));

        } else if (reader.name() == "plot") {
            m_concert.setOverview(reader.readElementText());

        } else if (reader.name() == "tagline") {
            m_concert.setTagline(reader.readElementText());

        } else if (reader.name() == "runtime") {
            m_concert.setRuntime(std::chrono::minutes(reader.readElementText().toInt()));

        } else if (reader.name() == "mpaa") {
            m_concert.setCertification(Certification(reader.readElementText()));

        } else if (reader.name() == "playcount") {
            m_concert.setPlayCount(reader.readElementText().toInt());

        } else if (reader.name() == "lastplayed") {
            m_concert.setLastPlayed(QDateTime::fromString(reader.readElementText(), "yyyy-MM-dd HH:mm:ss"));

        } else if (reader.name() == "trailer") {
            m_concert.setTrailer(QUrl(reader.readElementText()));

        } else if (reader.name() == "genre") {
            const QStringList genres = reader.readElementText().split(" / ", ElchSplitBehavior::SkipEmptyParts);
            for (const QString& genre : genres) {
                m_concert.addGenre(genre);
            }

        } else if (reader.name() == "tag") {
            m_concert.addTag(reader.readElementText());


        } else if (reader.name() == "uniqueid") {
            parseUniqueId(reader);

        } else if (reader.name() == "ratings") {
            parseRatings(reader);

        } else if (reader.name() == "rating") {
            // otherwise use "old" syntax:
            // <rating>10.0</rating>
            // <votes>10.0</votes>
            QString value = reader.readElementText();
            if (!value.isEmpty()) {
                oldStyleRating.rating = value.replace(",", ".").toDouble();
            }

        } else if (reader.name() == "votes") {
            QString value = reader.readElementText();
            if (!value.isEmpty()) {
                oldStyleRating.voteCount = value.replace(",", "").replace(".", "").toInt();
            }

        } else if (reader.name() == "thumb") {
            parsePoster(reader);

        } else if (reader.name() == "fanart") {
            parseFanart(reader);

        } else if (reader.name() == "fileinfo") {
            while (reader.readNextStartElement()) {
                if (reader.name() == "streamdetails") {
                    parseStreamDetails(reader);

                } else {
                    reader.skipCurrentElement();
                }
            }
        } else {
            reader.skipCurrentElement();
        }
    }

    if (oldStyleRating.rating > 0) {
        m_concert.ratings().push_back(oldStyleRating);
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
        if (reader.name() == "rating") {
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
                if (reader.name() == "value") {
                    rating.rating = reader.readElementText().replace(",", ".").toDouble();

                } else if (reader.name() == "votes") {
                    rating.voteCount = reader.readElementText().replace(",", "").replace(".", "").toInt();

                } else {
                    reader.skipCurrentElement();
                }
            }

            m_concert.ratings().push_back(rating);
            m_concert.setChanged(true);
        } else {
            reader.skipCurrentElement();
        }
    }
}

void ConcertXmlReader::parseFanart(QXmlStreamReader& reader)
{
    while (reader.readNextStartElement()) {
        if (reader.name() != "thumb") {
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

void ConcertXmlReader::parseStreamDetails(QXmlStreamReader& reader)
{
    auto* streamDetails = m_concert.streamDetails();

    int audioStreamNumber = 0;
    int subtitleStreamNumber = 0;

    while (reader.readNextStartElement()) {
        if (reader.name() == "video") {
            parseVideoStreamDetails(reader, streamDetails);

        } else if (reader.name() == "audio") {
            parseAudioStreamDetails(reader, audioStreamNumber, streamDetails);
            ++audioStreamNumber;

        } else if (reader.name() == "subtitle") {
            parseSubtitleStreamDetails(reader, subtitleStreamNumber, streamDetails);
            ++subtitleStreamNumber;

        } else {
            reader.skipCurrentElement();
        }
    }

    m_concert.setStreamDetailsLoaded(true);
}

void ConcertXmlReader::parseVideoStreamDetails(QXmlStreamReader& reader, StreamDetails* streamDetails)
{
    const auto readAndStore = [&reader, &streamDetails](StreamDetails::VideoDetails detail) {
        const QString value = reader.readElementText();
        if (!value.isEmpty()) {
            streamDetails->setVideoDetail(detail, value);
        }
    };

    while (reader.readNextStartElement()) {
        if (reader.name() == StreamDetails::detailToString(StreamDetails::VideoDetails::Codec)) {
            readAndStore(StreamDetails::VideoDetails::Codec);

        } else if (reader.name() == StreamDetails::detailToString(StreamDetails::VideoDetails::Aspect)) {
            readAndStore(StreamDetails::VideoDetails::Aspect);

        } else if (reader.name() == StreamDetails::detailToString(StreamDetails::VideoDetails::Width)) {
            readAndStore(StreamDetails::VideoDetails::Width);

        } else if (reader.name() == StreamDetails::detailToString(StreamDetails::VideoDetails::Height)) {
            readAndStore(StreamDetails::VideoDetails::Height);

        } else if (reader.name() == StreamDetails::detailToString(StreamDetails::VideoDetails::DurationInSeconds)) {
            readAndStore(StreamDetails::VideoDetails::DurationInSeconds);

        } else if (reader.name() == StreamDetails::detailToString(StreamDetails::VideoDetails::ScanType)) {
            readAndStore(StreamDetails::VideoDetails::ScanType);

        } else if (reader.name() == StreamDetails::detailToString(StreamDetails::VideoDetails::StereoMode)) {
            readAndStore(StreamDetails::VideoDetails::StereoMode);

        } else {
            reader.skipCurrentElement();
        }
    }
}

void ConcertXmlReader::parseAudioStreamDetails(QXmlStreamReader& reader, int streamNumber, StreamDetails* streamDetails)
{
    const auto readAndStore = [&](StreamDetails::AudioDetails detail) {
        const QString value = reader.readElementText();
        if (!value.isEmpty()) {
            streamDetails->setAudioDetail(streamNumber, detail, value);
        }
    };

    while (reader.readNextStartElement()) {
        if (reader.name() == StreamDetails::detailToString(StreamDetails::AudioDetails::Codec)) {
            readAndStore(StreamDetails::AudioDetails::Codec);

        } else if (reader.name() == StreamDetails::detailToString(StreamDetails::AudioDetails::Language)) {
            readAndStore(StreamDetails::AudioDetails::Language);

        } else if (reader.name() == StreamDetails::detailToString(StreamDetails::AudioDetails::Channels)) {
            readAndStore(StreamDetails::AudioDetails::Channels);

        } else {
            reader.skipCurrentElement();
        }
    }
}

void ConcertXmlReader::parseSubtitleStreamDetails(QXmlStreamReader& reader,
    int streamNumber,
    StreamDetails* streamDetails)
{
    const auto readAndStore = [&](StreamDetails::SubtitleDetails detail) {
        const QString value = reader.readElementText();
        if (!value.isEmpty()) {
            streamDetails->setSubtitleDetail(streamNumber, detail, value);
        }
    };

    while (reader.readNextStartElement()) {
        if (reader.name() == StreamDetails::detailToString(StreamDetails::SubtitleDetails::Language)) {
            readAndStore(StreamDetails::SubtitleDetails::Language);

        } else {
            reader.skipCurrentElement();
        }
    }
}

} // namespace kodi
} // namespace mediaelch
