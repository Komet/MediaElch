#pragma once

#include "globals/Globals.h"
#include "scrapers/tv_show/TheTvDb/ApiRequest.h"
#include "scrapers/tv_show/TheTvDb/Cache.h"
#include "scrapers/tv_show/TheTvDb/ShowParser.h"
#include "tv_shows/TvShow.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QUrl>

namespace thetvdb {

class ShowLoader : public QObject
{
    Q_OBJECT

public:
    ShowLoader(TvShow& show,
        QString language,
        QVector<TvShowScraperInfos> showInfosToLoad,
        QVector<TvShowScraperInfos> episodeInfosToLoad,
        TvShowUpdateType updateType,
        QObject* parent = nullptr);

    static const QVector<TvShowScraperInfos> scraperInfos;

    void loadShowAndEpisodes();
    void storeEpisodesInDatabase();
    QVector<TvShowEpisode*> mergeEpisodesToShow();

    /// Get the parser object that parses TheTvDb's json and stores (all)
    /// episodes separately.
    const ShowParser& parser() const { return m_parser; }

signals:
    void sigLoadDone();

private:
    enum class ApiShowDetails
    {
        INFOS,
        ACTORS
    };

    QVector<TvShowScraperInfos> m_loaded;
    bool m_episodesLoaded{false};

    TvShow& m_show;
    ApiRequest m_apiRequest;
    QVector<TvShowScraperInfos> m_infosToLoad;
    QVector<TvShowScraperInfos> m_episodeInfosToLoad;
    TvShowUpdateType m_updateType;
    ShowParser m_parser;

    void loadTvShow();
    void loadActors();
    void loadImages(TvShowScraperInfos imageType);
    void loadAndStoreEpisodes(ApiPage page);

    void checkIfDone();
    QUrl getFullUrl(const QString& suffix) const;
    QUrl getShowUrl(ApiShowDetails type) const;
    QUrl getImagesUrl(TvShowScraperInfos type) const;
    QUrl getEpisodesUrl(ApiPage page) const;

    void mergeEpisode(TvShowEpisode* episode);
    const TvShowEpisode* findLoadedEpisode(SeasonNumber season, EpisodeNumber episode);
};

} // namespace thetvdb
