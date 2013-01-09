#ifndef THETVDB_H
#define THETVDB_H

#include <QComboBox>
#include <QDomElement>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

#include "data/TvScraperInterface.h"

/**
 * @brief The TheTvDb class
 */
class TheTvDb : public TvScraperInterface
{
    Q_OBJECT
public:
    explicit TheTvDb(QObject *parent = 0);
    QString name();
    void search(QString searchStr);
    void loadTvShowData(QString id, TvShow *show, bool updateAllEpisodes, QList<int> infosToLoad);
    void loadTvShowEpisodeData(QString id, TvShowEpisode *episode, QList<int> infosToLoad);
    bool hasSettings();
    void loadSettings();
    void saveSettings();
    QMap<QString, QString> languages();
    QString language();
    void setLanguage(QString language);

signals:
    void sigSearchDone(QList<ScraperSearchResult>);
    void sigImagesLoaded(QString, QString);

private slots:
    void onMirrorsReady();
    void onSearchFinished();
    void onLoadFinished();
    void onEpisodeLoadFinished();
    void onActorsFinished();
    void onBannersFinished();

private:
    QString m_apiKey;
    QString m_language;
    QNetworkAccessManager m_qnam;
    QNetworkReply *m_mirrorsReply;
    QNetworkReply *m_searchReply;
    QNetworkReply *m_loadReply;
    QNetworkReply *m_episodeLoadReply;
    QNetworkReply *m_actorsReply;
    QNetworkReply *m_bannersReply;
    QStringList m_xmlMirrors;
    QStringList m_bannerMirrors;
    QStringList m_zipMirrors;
    TvShow *m_currentShow;
    TvShowEpisode *m_currentEpisode;
    bool m_updateAllEpisodes;
    QString m_currentId;
    QList<int> m_infosToLoad;

    QNetworkAccessManager *qnam();
    void setMirrors();
    QList<ScraperSearchResult> parseSearch(QString xml);
    void parseAndAssignInfos(QString xml, TvShow *show, bool updateAllEpisodes);
    void parseAndAssignActors(QString xml, TvShow *show);
    void parseAndAssignBanners(QString xml, TvShow *show);
    void parseAndAssignSingleEpisodeInfos(QDomElement elem, TvShowEpisode *episode);
};

#endif // THETVDB_H
