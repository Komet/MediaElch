#ifndef VIDEOBUSTER_H
#define VIDEOBUSTER_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QObject>
#include <QWidget>

#include "data/ScraperInterface.h"

/**
 * @brief The VideoBuster class
 */
class VideoBuster : public ScraperInterface
{
    Q_OBJECT
public:
    explicit VideoBuster(QObject *parent = 0);
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
    void backdropFinished();

private:
    Movie *m_currentMovie;
    QNetworkAccessManager m_qnam;
    QNetworkReply *m_searchReply;
    QNetworkReply *m_loadReply;
    QNetworkReply *m_backdropReply;
    QList<int> m_infosToLoad;
    QList<int> m_scraperSupports;

    QNetworkAccessManager *qnam();
    QList<ScraperSearchResult> parseSearch(QString html);
    void parseAndAssignInfos(QString html, Movie *movie, QList<int> infos);
};

#endif // VIDEOBUSTER_H
