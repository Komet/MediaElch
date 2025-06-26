#include "media_center/kodi/TvShowXmlWriter.h"

#include "data/tv_show/TvShow.h"
#include "globals/Helper.h"
#include "media_center/KodiXml.h"
#include "utils/Meta.h"

#include <QDomDocument>
#include <QJsonDocument>
#include <QJsonObject>

namespace mediaelch {
namespace kodi {

TvShowXmlWriterGeneric::TvShowXmlWriterGeneric(KodiVersion version, TvShow& tvShow) :
    TvShowXmlWriter(std::move(version)), m_show{tvShow}
{
}

QByteArray TvShowXmlWriterGeneric::getTvShowXml(bool testMode)
{
    // See <https://kodi.wiki/view/NFO_files/TV_shows>
    using namespace std::chrono_literals;

    QByteArray xmlContent;
    QXmlStreamWriter xml(&xmlContent);
    xml.setAutoFormatting(true);
    xml.writeStartDocument("1.0", true);

    xml.writeStartElement("tvshow");

    xml.writeTextElement("title", m_show.title());
    xml.writeTextElement("showtitle", m_show.showTitle());

    if (!m_show.sortTitle().isEmpty()) {
        xml.writeStartElement("sorttitle");
        xml.writeAttribute("clear", "true");
        xml.writeCharacters(m_show.sortTitle());
        xml.writeEndElement();
    }

    if (!m_show.originalTitle().isEmpty()) {
        xml.writeTextElement("originaltitle", m_show.originalTitle());
    }

    QString defaultId;

    // unique id: IMDb, TheTvDb and TMDB

    // one uniqueid is required
    // The first one of these IDs is the default:
    //  - TMDB
    //  - TvDb
    //  - IMDb
    //  - TvMaze
    bool hasDefault = false;
    if (m_show.tmdbId().isValid()) {
        xml.writeStartElement("uniqueid");
        if (!hasDefault) {
            xml.writeAttribute("default", "true");
            defaultId = m_show.tmdbId().toString();
            hasDefault = true;
        }
        xml.writeAttribute("type", "tmdb");
        xml.writeCharacters(m_show.tmdbId().toString());
        xml.writeEndElement();
    }
    if (m_show.imdbId().isValid()) {
        xml.writeStartElement("uniqueid");
        if (!hasDefault) {
            xml.writeAttribute("default", "true");
            defaultId = m_show.imdbId().toString();
            hasDefault = true;
        }
        xml.writeAttribute("type", "imdb");
        xml.writeCharacters(m_show.imdbId().toString());
        xml.writeEndElement();
    }
    if (m_show.tvmazeId().isValid()) {
        xml.writeStartElement("uniqueid");
        if (!hasDefault) {
            xml.writeAttribute("default", "true");
            defaultId = m_show.tvmazeId().toString();
            hasDefault = true;
        }
        xml.writeAttribute("type", "tvmaze");
        xml.writeCharacters(m_show.tvmazeId().toString());
        xml.writeEndElement();
    }
    if (m_show.tvdbId().isValid()) {
        xml.writeStartElement("uniqueid");
        if (!hasDefault) {
            xml.writeAttribute("default", "true");
            defaultId = m_show.tvdbId().toString();
            hasDefault = true;
        }
        xml.writeAttribute("type", "tvdb");
        xml.writeCharacters(m_show.tvdbId().toString());
        xml.writeEndElement();
    }

    if (hasDefault) {
        // id: Not used for Kodi import
        xml.writeTextElement("id", defaultId);

    } else {
        // fallback
        xml.writeComment(" No valid ID was defined - using internal DB ID as fallback ");
        xml.writeStartElement("uniqueid");
        if (!hasDefault) {
            xml.writeAttribute("default", "true");
            defaultId = QString::number(m_show.databaseId().toInt());
            hasDefault = true;
        }
        xml.writeAttribute("type", "mediaelch_fallback");
        xml.writeCharacters(QString::number(m_show.databaseId().toInt()));
        xml.writeEndElement();
    }

    Q_UNUSED(hasDefault); // "use" last value

    writeRatings(xml, m_show.ratings());

    xml.writeTextElement("userrating", QString::number(m_show.userRating()));
    xml.writeTextElement("top250", QString::number(m_show.top250()));
    xml.writeTextElement("episode", QString::number(m_show.episodes().count()));
    xml.writeTextElement("season", QString::number(m_show.seasons().count()));
    xml.writeTextElement("plot", m_show.overview());
    xml.writeTextElement("mpaa", m_show.certification().toString());
    xml.writeTextElement("premiered", m_show.firstAired().toString("yyyy-MM-dd"));
    if (version().toInt() < 20) {
        // Since Kodi v20, "year" should no longer be used, only "premiered".
        xml.writeTextElement("year", m_show.firstAired().toString("yyyy"));
    }
    xml.writeTextElement("dateadded", m_show.dateAdded().toString("yyyy-MM-dd HH:mm:ss"));
    xml.writeTextElement("status", m_show.status());

    KodiXml::writeStringsAsOneTagEach(xml, "studio", m_show.networks());

    if (m_show.runtime() > 0min) {
        xml.writeTextElement("runtime", QString::number(m_show.runtime().count()));
    }

    // TODO: add trailer support
    xml.writeTextElement("trailer", "");

    for (auto namedSeason = m_show.seasonNameMappings().constBegin();
        namedSeason != m_show.seasonNameMappings().constEnd();
        ++namedSeason) {
        xml.writeStartElement("namedseason");
        xml.writeAttribute("number", namedSeason.key().toString());
        xml.writeCharacters(namedSeason.value());
        xml.writeEndElement();
    }

    if (version().toInt() < 19) {
        // The episode guide was never really properly described until Kodi version 19.5.
        // Prior to that, we used either the TMDB ID or a TheTvDb URL.
        if (m_show.tmdbId().isValid()) {
            // Prefer TMDB episode guide to TvDb one.
            xml.writeStartElement("episodeguide");
            xml.writeCharacters(m_show.tmdbId().toString());
            xml.writeEndElement();

        } else if (m_show.tvdbId().isValid()) {
            // Always write the episodeGuideUrl using a fixed URL. The apikey is
            // fixed and has been taken from https://forum.kodi.tv/showthread.php?tid=323588
            // See https://github.com/Komet/MediaElch/issues/652
            //
            // TODO:
            // There may be future changes to the episode guide url:
            // See https://github.com/Komet/MediaElch/issues/888
            xml.writeStartElement("episodeguide");
            xml.writeStartElement("url");
            xml.writeAttribute("post", "yes");
            xml.writeAttribute("cache", "auth.json");
            QString url =
                QStringLiteral(R"(https://api.thetvdb.com/login?{"apikey":"%1","id":%2}|Content-Type=application/json)")
                    .arg("439DFEBA9D3059C6", m_show.tvdbId().toString());
            xml.writeCharacters(url);
            xml.writeEndElement();
            xml.writeEndElement();
        }
    } else {
        // Kodi Python scrapers are only passed the episodeguide to retrieve the list of
        // episodes, not <uniqueid>s, see https://github.com/xbmc/xbmc/issues/17341,
        // that's why we "duplicate" the IDs for v19 and later.
        // Example:
        // <episodeguide>{"tvmaze": "428", "tvrage": "2610", "tvdb": "71035", "tmdb": "2426", "imdb":
        // "tt0162065"}</episodeguide>
        QJsonObject ids;
        if (m_show.tvmazeId().isValid()) {
            ids.insert("tvmaze", m_show.tvmazeId().toString());
        }
        if (m_show.tvdbId().isValid()) {
            ids.insert("tvdb", m_show.tvdbId().toString());
        }
        if (m_show.tmdbId().isValid()) {
            ids.insert("tmdb", m_show.tmdbId().toString());
        }
        if (m_show.imdbId().isValid()) {
            ids.insert("imdb", m_show.imdbId().toString());
        }
        QJsonDocument doc;
        doc.setObject(ids);
        QString episodeguide = doc.toJson(QJsonDocument::Compact);

        xml.writeStartElement("episodeguide");
        xml.writeCharacters(episodeguide);
        xml.writeEndElement();
    }

    KodiXml::writeStringsAsOneTagEach(xml, "genre", m_show.genres());
    KodiXml::writeStringsAsOneTagEach(xml, "tag", m_show.tags());

    if (writeThumbUrlsToNfo()) {
        const auto& posters = m_show.posters();
        for (const Poster& poster : posters) {
            xml.writeStartElement("thumb");
            if (!poster.language.isEmpty()) {
                xml.writeAttribute("language", poster.language);
            }
            QString aspect = poster.aspect.isEmpty() ? "poster" : poster.aspect;
            xml.writeAttribute("aspect", aspect);
            if (!poster.thumbUrl.isEmpty()) {
                xml.writeAttribute("preview", poster.thumbUrl.toString());
            }
            if (poster.season != SeasonNumber::NoSeason) {
                xml.writeAttribute("type", "season");
                xml.writeAttribute("season", poster.season.toString());
            }
            xml.writeCharacters(poster.originalUrl.toString());
            xml.writeEndElement();
        }

        const auto& banners = m_show.banners();
        for (const Poster& banner : banners) {
            xml.writeStartElement("thumb");
            QString aspect = banner.aspect.isEmpty() ? "banner" : banner.aspect;
            if (!banner.language.isEmpty()) {
                xml.writeAttribute("language", banner.language);
            }
            xml.writeAttribute("aspect", aspect);
            if (!banner.thumbUrl.isEmpty()) {
                xml.writeAttribute("preview", banner.thumbUrl.toString());
            }
            xml.writeCharacters(banner.originalUrl.toString());
            xml.writeEndElement();
        }

        const auto& seasonPosters = m_show.seasonPosters(SeasonNumber::NoSeason, true);
        for (const Poster& poster : seasonPosters) {
            if (poster.season != SeasonNumber::NoSeason) {
                xml.writeStartElement("thumb");
                if (!poster.language.isEmpty()) {
                    xml.writeAttribute("language", poster.language);
                }
                xml.writeAttribute("aspect", poster.aspect.isEmpty() ? "poster" : poster.aspect);
                xml.writeAttribute("season", poster.season.toString());
                if (!poster.thumbUrl.isEmpty()) {
                    xml.writeAttribute("preview", poster.thumbUrl.toString());
                }
                xml.writeAttribute("type", "season");
                xml.writeCharacters(poster.originalUrl.toString());
                xml.writeEndElement();
            }
        }

        const auto& seasonBanners = m_show.seasonBanners(SeasonNumber::NoSeason, true);
        for (const Poster& banner : seasonBanners) {
            if (banner.season != SeasonNumber::NoSeason) {
                xml.writeStartElement("thumb");
                if (!banner.language.isEmpty()) {
                    xml.writeAttribute("language", banner.language);
                }
                xml.writeAttribute("aspect", banner.aspect.isEmpty() ? "poster" : banner.aspect);
                xml.writeAttribute("season", banner.season.toString());
                if (!banner.thumbUrl.isEmpty()) {
                    xml.writeAttribute("preview", banner.thumbUrl.toString());
                }
                xml.writeAttribute("type", "season");
                xml.writeCharacters(banner.originalUrl.toString());
                xml.writeEndElement();
            }
        }

        if (!m_show.backdrops().isEmpty()) {
            xml.writeStartElement("fanart");
            const auto& backdrops = m_show.backdrops();
            for (const Poster& poster : backdrops) {
                xml.writeStartElement("thumb");
                xml.writeAttribute("preview", poster.thumbUrl.toString());

                if (poster.originalSize.isValid()) {
                    const int w = poster.originalSize.width();
                    const int h = poster.originalSize.height();
                    xml.writeAttribute("dim", QString("%1x%2").arg(w).arg(h));
                }

                xml.writeCharacters(poster.originalUrl.toString());
                xml.writeEndElement();
            }
            xml.writeEndElement();
        }
    }

    writeActors(xml, m_show.actors());

    if (!testMode) {
        addMediaelchGeneratorTag(xml);
    }

    xml.writeEndElement();
    return xmlContent;
}

} // namespace kodi
} // namespace mediaelch
