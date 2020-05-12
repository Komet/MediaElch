#include "EpisodeLoader.h"

#include "scrapers/tv_show/thetvdb/ApiRequest.h"
#include "scrapers/tv_show/thetvdb/EpisodeParser.h"
#include "settings/Settings.h"
#include "tv_shows/TvShowEpisode.h"

#include <QObject>
#include <QString>
#include <utility>

namespace thetvdb {

const QSet<ShowScraperInfos> EpisodeLoader::scraperInfos = {
    ShowScraperInfos::Director,
    ShowScraperInfos::Title,
    ShowScraperInfos::FirstAired,
    ShowScraperInfos::Overview,
    ShowScraperInfos::Rating,
    ShowScraperInfos::Writer,
    ShowScraperInfos::Thumbnail,
    ShowScraperInfos::Certification // loaded by IMDb
};

EpisodeLoader::EpisodeLoader(TvDbId showId,
    TvShowEpisode& episode,
    QString language,
    QSet<ShowScraperInfos> infosToLoad,
    QObject* parent) :
    QObject(parent),
    m_showId{std::move(showId)},
    m_episode{episode},
    m_apiRequest(language),
    m_infosToLoad{std::move(infosToLoad)}
{
    setParent(parent);
}

void EpisodeLoader::loadData()
{
    if (m_episode.tvdbId().isValid()) {
        loadEpisode();
        return;
    }
    // We have to get the TvDbId first to get the episode's id.
    loadSeason();
}

void EpisodeLoader::loadSeason()
{
    qDebug() << "[TheTvDb][EpisodeLoader] Have to load season first.";
    m_apiRequest.sendGetRequest(getSeasonUrl(), [this](QString json) {
        // TODO: It is possible that results are paginated.
        EpisodeParser parser(m_episode, m_infosToLoad);
        parser.parseIdFromSeason(json);
        loadEpisode();
    });
}

void EpisodeLoader::loadEpisode()
{
    m_apiRequest.sendGetRequest(getEpisodeUrl(), [this](QString json) {
        EpisodeParser parser(m_episode, m_infosToLoad);
        parser.parseInfos(json);
        emit sigLoadDone();
    });
}

QUrl EpisodeLoader::getEpisodeUrl() const
{
    return ApiRequest::getFullUrl(QStringLiteral("/episodes/%1").arg(m_episode.tvdbId().toString()));
}

QUrl EpisodeLoader::getSeasonUrl() const
{
    const QString seasonType = Settings::instance()->tvShowDvdOrder() ? "dvdSeason" : "airedSeason";
    return ApiRequest::getFullUrl(QStringLiteral("/series/%1/episodes/query?%2=%3")
                                      .arg(m_showId.toString(), seasonType, m_episode.season().toString()));
}

} // namespace thetvdb
