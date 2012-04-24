#ifndef THETVDB_H
#define THETVDB_H

#include <QComboBox>
#include <QDomElement>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

#include "data/TvScraperInterface.h"

class TheTvDb : public TvScraperInterface
{
    Q_OBJECT
public:
    explicit TheTvDb(QObject *parent = 0);
    QString name();
    void search(QString searchStr);
    void loadTvShowData(QString id, TvShow *show);
    void loadTvShowEpisodeData(QString id, TvShowEpisode *episode);
    bool hasSettings();
    void loadSettings();
    void saveSettings();
    QWidget* settingsWidget();

signals:
    void sigSearchDone(QList<ScraperSearchResult>);

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
    QComboBox *m_settingsLanguageCombo;
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
    QString m_currentId;

    QNetworkAccessManager *qnam();
    void setMirrors();
    QList<ScraperSearchResult> parseSearch(QString xml);
    void parseAndAssignInfos(QString xml, TvShow *show);
    void parseAndAssignActors(QString xml, TvShow *show);
    void parseAndAssignBanners(QString xml, TvShow *show);
    void parseAndAssignSingleEpisodeInfos(QDomElement elem, TvShowEpisode *episode);
};

#endif // THETVDB_H
