#ifndef TMDB_H
#define TMDB_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QMutex>
#include <QObject>

#include "data/ScraperInterface.h"

/**
 * @brief The TMDb class
 */
class TMDb : public ScraperInterface
{
    Q_OBJECT
public:
    explicit TMDb(QObject *parent = 0);
    ~TMDb();
    QString name();
    void search(QString searchStr);
    void loadData(QString id, Movie *movie, QList<int> infos);
    bool hasSettings();
    void loadSettings();
    void saveSettings();
    QList<int> scraperSupports();
    QMap<QString, QString> languages();
    QString language();
    void setLanguage(QString language);

signals:
    void searchDone(QList<ScraperSearchResult>);

private slots:
    void searchFinished();
    void loadFinished();
    void loadCastsFinished();
    void loadTrailersFinished();
    void loadImagesFinished();
    void loadReleasesFinished();
    void setupFinished();

private:
    QString m_apiKey;
    QNetworkAccessManager m_qnam;
    QNetworkReply *m_searchReply;
    QNetworkReply *m_loadReply;
    QNetworkReply *m_castsReply;
    QNetworkReply *m_trailersReply;
    QNetworkReply *m_imagesReply;
    QNetworkReply *m_releasesReply;
    QNetworkReply *m_setupReply;
    Movie *m_currentMovie;
    QString m_currentId;
    QString m_language;
    QList<ScraperSearchResult> m_results;
    QString m_searchString;
    QNetworkAccessManager *qnam();
    QList<ScraperSearchResult> parseSearch(QString json, int *nextPage);
    QString m_baseUrl;
    QMutex m_mutex;
    bool m_loadDoneFired;
    enum Data {
        DataInfos, DataCasts, DataTrailers, DataImages, DataReleases
    };
    QList<Data> m_loadsLeft;
    QList<int> m_infosToLoad;
    QList<int> m_scraperSupports;

    void setup();
    void parseAndAssignInfos(QString json, Movie *movie, QList<int> infos);
    void checkDownloadsFinished();
};

#endif // TMDB_H
