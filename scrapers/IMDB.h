#ifndef IMDB_H
#define IMDB_H

#include "QNetworkAccessManager"
#include "QNetworkReply"
#include "data/ScraperInterface.h"

class IMDB : public ScraperInterface
{
    Q_OBJECT
public:
    explicit IMDB(QObject *parent = 0);
    QString name();
    QString identifier();
    void search(QString searchStr);
    void loadData(QMap<ScraperInterface*, QString> ids, Movie *movie, QList<int> infos);
    bool hasSettings();
    void loadSettings(QSettings &settings);
    void saveSettings(QSettings &settings);
    QList<int> scraperSupports();
    QList<int> scraperNativelySupports();
    QWidget *settingsWidget();
    bool isAdult();
    void parseAndAssignInfos(QString html, Movie *movie, QList<int> infos);

signals:
    void searchDone(QList<ScraperSearchResult>);

private slots:
    void onSearchFinished();
    void onSearchIdFinished();
    void onLoadFinished();
    void onPosterLoadFinished();

private:
    QNetworkAccessManager *qnam();
    QList<ScraperSearchResult> parseSearch(QString html);
    void parseAndAssignPoster(QString html, Movie *movie, QList<int> infos);
    QString parsePosters(QString html);

    QNetworkAccessManager m_qnam;
    QList<int> m_scraperSupports;
};

#endif // IMDB_H
