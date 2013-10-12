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
    QStringList m_xmlMirrors;
    QStringList m_bannerMirrors;
    QStringList m_zipMirrors;

    QNetworkAccessManager *qnam();
    void setMirrors();
    QList<ScraperSearchResult> parseSearch(QString xml);
    void parseAndAssignInfos(QString xml, TvShow *show, TvShowUpdateType updateType, QList<int> infosToLoad);
    void parseAndAssignActors(QString xml, TvShow *show);
    void parseAndAssignBanners(QString xml, TvShow *show, TvShowUpdateType updateType, QList<int> infosToLoad);
    void parseAndAssignSingleEpisodeInfos(QDomElement elem, TvShowEpisode *episode, QList<int> infosToLoad);
    QComboBox *m_box;
    QWidget *m_widget;
};

#endif // THETVDB_H
