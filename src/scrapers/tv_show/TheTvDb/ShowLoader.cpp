#include "ShowLoader.h"

#include "globals/Globals.h"
#include "globals/Manager.h"
#include "scrapers/tv_show/TheTvDb/ApiRequest.h"
#include "scrapers/tv_show/TheTvDb/EpisodeLoader.h"
#include "scrapers/tv_show/TheTvDb/ShowLoader.h"
#include "tv_shows/TvShow.h"

#include <QObject>
#include <QString>
#include <QVector>
#include <utility>

namespace thetvdb {

// All infos that this API can scrape.
const QVector<TvShowScraperInfos> ShowLoader::scraperInfos = {TvShowScraperInfos::Actors,
    TvShowScraperInfos::Certification,
    TvShowScraperInfos::FirstAired,
    TvShowScraperInfos::Genres,
    TvShowScraperInfos::Network,
    TvShowScraperInfos::Overview,
    TvShowScraperInfos::Rating,
    TvShowScraperInfos::Title,
    TvShowScraperInfos::Runtime,
    TvShowScraperInfos::Status,
    TvShowScraperInfos::Fanart,
    TvShowScraperInfos::Poster,
    TvShowScraperInfos::SeasonPoster,
    TvShowScraperInfos::SeasonBanner,
    TvShowScraperInfos::Banner};

/// @brief Load TvShow data from TheTvDb. See ShowLoader::scraperInfos.
/// @param show               TvShow to load and store information to. Must have TheTvDb ID set.
/// @param language           TheTvDb language string, e.g. en or de-DE
/// @param showInfosToLoad    Show information to load. If no item is given, only basic information will be scraped.
/// @param episodeInfosToLoad Episode information to load.
/// @param updateType         Tells whether to update only the show, all episodes, new episodes, etc.
ShowLoader::ShowLoader(TvShow& show,
    QString language,
    QVector<TvShowScraperInfos> showInfosToLoad,
    QVector<TvShowScraperInfos> episodeInfosToLoad,
    TvShowUpdateType updateType,
    QObject* parent) :
    QObject(parent),
    m_show{show},
    m_apiRequest(language),
    m_episodeInfosToLoad{std::move(episodeInfosToLoad)},
    m_updateType{updateType},
    m_parser(show, showInfosToLoad)
{
    setParent(parent);

    // Save only information that we can actually scrape
    m_infosToLoad = [&showInfosToLoad]() {
        QVector<TvShowScraperInfos> infos;
        for (const auto info : showInfosToLoad) {
            if (ShowLoader::scraperInfos.contains(info)) {
                infos.append(info);
            }
        }
        return infos;
    }();
}

void ShowLoader::loadShowAndEpisodes()
{
    if (!m_show.tvdbId().isValid()) {
        qWarning() << "[TheTvDb][ShowLoader] Can't load show without TheTvDb id";
        emit sigLoadDone();
        return;
    }

    if (isEpisodeUpdateType(m_updateType)) {
        loadAndStoreEpisodes(ApiPage{1});
    } else {
        m_episodesLoaded = true;
    }

    if (!isShowUpdateType(m_updateType)) {
        // We don't need to load any show related information.
        m_loaded = m_infosToLoad;
        checkIfDone();
        return;
    }

    m_show.setId(m_show.tvdbId());

    // TV Show information and episodes are always loaded.
    loadTvShow();

    if (m_infosToLoad.contains(TvShowScraperInfos::Actors)) {
        loadActors();
    }
    if (m_infosToLoad.contains(TvShowScraperInfos::Fanart)) {
        loadImages(TvShowScraperInfos::Fanart);
    }
    if (m_infosToLoad.contains(TvShowScraperInfos::Poster)) {
        loadImages(TvShowScraperInfos::Poster);
    }
    if (m_infosToLoad.contains(TvShowScraperInfos::SeasonPoster)) {
        loadImages(TvShowScraperInfos::SeasonPoster);
    }
    if (m_infosToLoad.contains(TvShowScraperInfos::SeasonBanner)) {
        loadImages(TvShowScraperInfos::SeasonBanner);
    }
    if (m_infosToLoad.contains(TvShowScraperInfos::Banner)
        || m_infosToLoad.contains(TvShowScraperInfos::SeasonBanner)) {
        loadImages(TvShowScraperInfos::Banner);
    }
}

void ShowLoader::loadTvShow()
{
    const auto setInfosLoaded = [this]() {
        const QVector<TvShowScraperInfos> availableScraperInfos = {TvShowScraperInfos::Certification,
            TvShowScraperInfos::FirstAired,
            TvShowScraperInfos::Genres,
            TvShowScraperInfos::Network,
            TvShowScraperInfos::Overview,
            TvShowScraperInfos::Rating,
            TvShowScraperInfos::Title,
            TvShowScraperInfos::Runtime,
            TvShowScraperInfos::Status};

        for (const auto loaded : availableScraperInfos) {
            if (m_infosToLoad.contains(loaded)) {
                m_loaded.append(loaded);
            }
        }
    };

    m_apiRequest.sendGetRequest(getShowUrl(ApiShowDetails::INFOS), [this, setInfosLoaded](QString json) {
        // We need to add the loaded information but may not want to actually store the show's information.
        if (isShowUpdateType(m_updateType)) {
            m_parser.parseInfos(json);
        }
        setInfosLoaded();
        checkIfDone();
    });
}

void ShowLoader::loadActors()
{
    m_apiRequest.sendGetRequest(getShowUrl(ApiShowDetails::ACTORS), [this](QString json) {
        m_parser.parseActors(json);
        m_loaded.append(TvShowScraperInfos::Actors);
        checkIfDone();
    });
}

void ShowLoader::loadImages(TvShowScraperInfos imageType)
{
    m_apiRequest.sendGetRequest(getImagesUrl(imageType), [this, imageType](QString json) {
        m_parser.parseImages(json);
        m_loaded.append(imageType);
        checkIfDone();
    });
}

void ShowLoader::loadAndStoreEpisodes(ApiPage page)
{
    m_apiRequest.sendGetRequest(getEpisodesUrl(page), [this](QString json) {
        Paginate p = m_parser.parseEpisodes(json, m_episodeInfosToLoad);
        if (p.hasNextPage()) {
            return loadAndStoreEpisodes(p.next);
        }

        m_episodesLoaded = true;
        checkIfDone();
    });
}

void ShowLoader::storeEpisodesInDatabase()
{
    Database* const database = Manager::instance()->database();
    const int showsSettingsId = database->showsSettingsId(&m_show);
    database->clearEpisodeList(showsSettingsId);

    for (auto& episode : m_parser.episodes()) {
        database->addEpisodeToShowList(episode.get(), showsSettingsId, episode->tvdbId());
    }

    database->cleanUpEpisodeList(showsSettingsId);
}

/**
 * @brief Merge downloaded episodes with the episodes existing in ShowLoader::m_show
 *        Uses ShowLoader::m_updateType to determine what information should be stored.
 */
QVector<TvShowEpisode*> ShowLoader::mergeEpisodesToShow()
{
    QVector<TvShowEpisode*> updatedEpisodes;

    const bool loadNew = isNewEpisodeUpdateType(m_updateType);
    const bool loadAll = isAllEpisodeUpdateType(m_updateType);

    for (TvShowEpisode* episode : m_show.episodes()) {
        if (loadNew && !episode->infoLoaded()) {
            mergeEpisode(episode);
            updatedEpisodes.append(episode);
        }
        if (loadAll && !episode->isDummy()) {
            // Dummy episodes are loaded by ShowLoader. We only load detailed information
            // for episodes that are real.
            mergeEpisode(episode);
            updatedEpisodes.append(episode);
        }
    }

    return updatedEpisodes;
}

void ShowLoader::checkIfDone()
{
    if (m_loaded.size() >= m_infosToLoad.size() && m_episodesLoaded) {
        emit sigLoadDone();
    }
}

QUrl ShowLoader::getShowUrl(ApiShowDetails type) const
{
    const QString typeStr = [type]() {
        switch (type) {
        case ApiShowDetails::ACTORS: return QStringLiteral("/actors");
        case ApiShowDetails::INFOS: return QString{};
        }
        qWarning() << "[TheTvDb][ShowLoader] Unknown ApiShowDetails";
        return QString{};
    }();

    return ApiRequest::getFullUrl(QStringLiteral("/series/%1%2").arg(m_show.tvdbId().toString(), typeStr));
}

// See: TvShowScraperInfos
QUrl ShowLoader::getImagesUrl(TvShowScraperInfos type) const
{
    const QString typeStr = [type]() {
        switch (type) {
        case TvShowScraperInfos::Fanart: return QStringLiteral("fanart");
        case TvShowScraperInfos::Poster: return QStringLiteral("poster");
        case TvShowScraperInfos::SeasonPoster: return QStringLiteral("season");
        case TvShowScraperInfos::SeasonBanner: return QStringLiteral("seasonwide");
        case TvShowScraperInfos::Banner: return QStringLiteral("series");
        default: qWarning() << "[TheTvDb] Invalid image type"; return QStringLiteral("invalid");
        }
    }();

    return ApiRequest::getFullUrl(
        QStringLiteral("/series/%1/images/query?keyType=%2").arg(m_show.tvdbId().toString(), typeStr));
}

QUrl ShowLoader::getEpisodesUrl(ApiPage page) const
{
    return ApiRequest::getFullUrl(
        QStringLiteral("/series/%1/episodes?page=%2").arg(m_show.tvdbId().toString(), QString::number(page)));
}

void ShowLoader::mergeEpisode(TvShowEpisode* episode)
{
    if (episode == nullptr) {
        qWarning() << "[TheTvDb][ShowLoader] mergeEpisode: episode is nullptr";
        return;
    }

    const auto* loadedEpisode = findLoadedEpisode(episode->season(), episode->episode());
    if (loadedEpisode == nullptr) {
        return;
    }

    episode->setTvdbId(loadedEpisode->tvdbId());

    episode->setDisplaySeason(loadedEpisode->displaySeason());
    episode->setDisplayEpisode(loadedEpisode->displayEpisode());

    if (loadedEpisode->imdbId().isValid()) {
        episode->setImdbId(loadedEpisode->imdbId());
    }
    if (m_episodeInfosToLoad.contains(TvShowScraperInfos::Director) && !loadedEpisode->directors().isEmpty()) {
        episode->setDirectors(loadedEpisode->directors());
    }
    if (m_episodeInfosToLoad.contains(TvShowScraperInfos::Title) && !loadedEpisode->name().isEmpty()) {
        episode->setName(loadedEpisode->name());
    }
    if (m_episodeInfosToLoad.contains(TvShowScraperInfos::FirstAired) && loadedEpisode->firstAired().isValid()) {
        episode->setFirstAired(loadedEpisode->firstAired());
    }
    if (m_episodeInfosToLoad.contains(TvShowScraperInfos::Overview) && !loadedEpisode->overview().isEmpty()) {
        episode->setOverview(loadedEpisode->overview());
    }
    if (m_episodeInfosToLoad.contains(TvShowScraperInfos::Rating)) {
        episode->ratings() = loadedEpisode->ratings();
    }
    if (m_episodeInfosToLoad.contains(TvShowScraperInfos::Writer) && !loadedEpisode->writers().isEmpty()) {
        episode->setWriters(loadedEpisode->writers());
    }
    if (m_episodeInfosToLoad.contains(TvShowScraperInfos::Thumbnail) && !loadedEpisode->thumbnail().isEmpty()) {
        episode->setThumbnail(loadedEpisode->thumbnail());
    }

    if (m_episodeInfosToLoad.contains(TvShowScraperInfos::Tags) && !loadedEpisode->thumbnail().isEmpty()) {
        episode->setThumbnail(loadedEpisode->thumbnail());
    }
}

const TvShowEpisode* ShowLoader::findLoadedEpisode(SeasonNumber season, EpisodeNumber episode)
{
    const auto& episodes = m_parser.episodes();
    for (const auto& cur_episode : episodes) {
        if (cur_episode->season() == season && cur_episode->episode() == episode) {
            return cur_episode.get();
        }
    }
    return nullptr;
}

} // namespace thetvdb
