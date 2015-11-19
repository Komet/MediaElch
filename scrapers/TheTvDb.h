#ifndef THETVDB_H
#define THETVDB_H

#include <QComboBox>
#include <QDomElement>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

#include "data/TvScraperInterface.h"
#include "scrapers/IMDB.h"

/**
 * @brief The TheTvDb class
 */
class TheTvDb : public TvScraperInterface
{
    Q_OBJECT
public:
    explicit TheTvDb(QObject *parent = 0);
    QString name();
    QString identifier();
    void search(QString searchStr);
    void loadTvShowData(QString id, TvShow *show, TvShowUpdateType updateType, QList<int> infosToLoad);
    void loadTvShowEpisodeData(QString id, TvShowEpisode *episode, QList<int> infosToLoad);
    bool hasSettings();
    void loadSettings(QSettings &settings);
    void saveSettings(QSettings &settings);
    QWidget *settingsWidget();
    void fillDatabaseWithAllEpisodes(QString xml, TvShow *show);
    QString apiKey();
    QString language();

signals:
    void sigSearchDone(QList<ScraperSearchResult>);
    void sigImagesLoaded(QString, QString);
    void sigLoadProgress(TvShow*, int, int);

private slots:
    void onMirrorsReady();
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
    struct CacheElement {
        QDateTime date;
        QString data;
    };

    QString m_apiKey;
    QString m_language;
    QNetworkAccessManager m_qnam;
    QStringList m_xmlMirrors;
    QStringList m_bannerMirrors;
    QStringList m_zipMirrors;
    QComboBox *m_box;
    QWidget *m_widget;
    QMap<QUrl, CacheElement> m_cache;
    IMDB *m_imdb;
    Movie *m_dummyMovie;
    QList<int> m_movieInfos;

    QNetworkAccessManager *qnam();
    void setMirrors();
    QList<ScraperSearchResult> parseSearch(QString xml);
    void parseAndAssignInfos(QString xml, TvShow *show, TvShowUpdateType updateType, QList<int> infosToLoad, QList<TvShowEpisode*> &updatedEpisodes);
    void parseAndAssignActors(QString xml, TvShow *show);
    void parseAndAssignBanners(QString xml, TvShow *show, TvShowUpdateType updateType, QList<int> infosToLoad);
    void parseAndAssignSingleEpisodeInfos(QDomElement elem, TvShowEpisode *episode, QList<int> infosToLoad);
    void parseAndAssignImdbInfos(QString xml, TvShow *show, TvShowUpdateType updateType, QList<int> infosToLoad);
    void parseAndAssignImdbInfos(QString xml, TvShowEpisode *episode, QList<int> infosToLoad);
    void parseEpisodeXml(QString msg, TvShowEpisode *episode, QList<int> infos);
    bool shouldLoadImdb(QList<int> infosToLoad);
    bool shouldLoadFromImdb(int info, QList<int> infosToLoad);
    void getAiredSeasonAndEpisode(QString xml, TvShowEpisode *episode, int &seasonNumber, int &episodeNumber);
    QString getImdbIdForEpisode(QString html, int episodeNumber);
    bool processEpisodeData(QString msg, TvShowEpisode *episode, QList<int> infos);
    void loadEpisodes(TvShow *show, QList<TvShowEpisode*> episodes, QList<int> infosToLoad);
};

#endif // THETVDB_H
