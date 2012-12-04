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
    void onSearchFinished();
    void onLoadFinished();

private:
    QNetworkAccessManager *qnam();
    QList<ScraperSearchResult> parseSearch(QString json);
    void parseAndAssignInfos(QString json, Movie *movie, QList<int> infos);

    QNetworkAccessManager m_qnam;
    QNetworkReply *m_searchReply;
    QNetworkReply *m_loadReply;
    QList<int> m_infosToLoad;
    QList<int> m_scraperSupports;
    Movie *m_currentMovie;
};

#endif // IMDB_H
