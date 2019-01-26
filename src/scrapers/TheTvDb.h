#pragma once

#include "data/EpisodeNumber.h"
#include "data/ImdbId.h"
#include "data/SeasonNumber.h"
#include "data/TvScraperInterface.h"

#include <QComboBox>
#include <QDomElement>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

class IMDB;
class Movie;

/**
 * @brief The TheTvDb class
 */
class TheTvDb : public TvScraperInterface
{
    Q_OBJECT
public:
    explicit TheTvDb(QObject *parent = nullptr);
    QString name() const override;
    QString identifier() const override;
    void search(QString searchStr) override;
    void loadTvShowData(TvDbId id,
        TvShow *show,
        TvShowUpdateType updateType,
        QVector<TvShowScraperInfos> infosToLoad) override;
    void loadTvShowEpisodeData(TvDbId id, TvShowEpisode *episode, QVector<TvShowScraperInfos> infosToLoad) override;
    bool hasSettings() const override;
    void loadSettings(const ScraperSettings &settings) override;
    void saveSettings(ScraperSettings &settings) override;
    QWidget *settingsWidget() override;
    void fillDatabaseWithAllEpisodes(QString xml, TvShow *show);
    QString apiKey();
    QString language();

signals:
    void sigSearchDone(QVector<ScraperSearchResult>) override;
    void sigImagesLoaded(QString, QString);
    void sigLoadProgress(TvShow *, int, int);

private slots:
    void onSearchFinished();
    void onLoadFinished();
    void onEpisodeLoadFinished();
    void onActorsFinished();
    void onBannersFinished();
    void onImdbFinished();
    void onImdbSeasonFinished();
    void onImdbEpisodeFinished();
    void onEpisodesImdbSeasonFinished();
    void onEpisodesImdbEpisodeFinished();

private:
    struct CacheElement
    {
        QDateTime date;
        QString data;
    };

    QString m_apiKey;
    QString m_language;
    QNetworkAccessManager m_qnam;
    QString m_mirror;
    QComboBox *m_box;
    QWidget *m_widget;
    QMap<QUrl, CacheElement> m_cache;
    IMDB *m_imdb;
    Movie *m_dummyMovie;
    QVector<MovieScraperInfos> m_movieInfos;

    QNetworkAccessManager *qnam();
    QVector<ScraperSearchResult> parseSearch(QString xml);
    void parseAndAssignInfos(QString xml,
        TvShow *show,
        TvShowUpdateType updateType,
        QVector<TvShowScraperInfos> infosToLoad,
        QVector<TvShowEpisode *> &updatedEpisodes);
    void parseAndAssignActors(QString xml, TvShow *show);
    void parseAndAssignBanners(QString xml,
        TvShow *show,
        TvShowUpdateType updateType,
        QVector<TvShowScraperInfos> infosToLoad);
    void
    parseAndAssignSingleEpisodeInfos(QDomElement elem, TvShowEpisode *episode, QVector<TvShowScraperInfos> infosToLoad);
    void parseAndAssignImdbInfos(QString xml,
        TvShow *show,
        TvShowUpdateType updateType,
        QVector<TvShowScraperInfos> infosToLoad);
    void parseAndAssignImdbInfos(QString xml, TvShowEpisode *episode, QVector<TvShowScraperInfos> infosToLoad);
    void parseEpisodeXml(QString msg, TvShowEpisode *episode, QVector<TvShowScraperInfos> infos);
    bool shouldLoadImdb(QVector<TvShowScraperInfos> infosToLoad);
    bool shouldLoadFromImdb(TvShowScraperInfos info, QVector<TvShowScraperInfos> infosToLoad);
    void getAiredSeasonAndEpisode(QString xml,
        TvShowEpisode *episode,
        SeasonNumber &seasonNumber,
        EpisodeNumber &episodeNumber);
    ImdbId getImdbIdForEpisode(QString html, EpisodeNumber episodeNumber);
    bool processEpisodeData(QString msg, TvShowEpisode *episode, QVector<TvShowScraperInfos> infos);
    void loadEpisodes(TvShow *show, QVector<TvShowEpisode *> episodes, QVector<TvShowScraperInfos> infosToLoad);
};
