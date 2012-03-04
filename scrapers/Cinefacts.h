#ifndef CINEFACTS_H
#define CINEFACTS_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QQueue>
#include <QObject>
#include <QWidget>

#include "data/ScraperInterface.h"

class Cinefacts : public ScraperInterface
{
    Q_OBJECT
public:
    explicit Cinefacts(QObject *parent = 0);
    QString name();
    void search(QString searchStr);
    void loadData(QString id, Movie *movie);
    bool hasSettings();
    void loadSettings();
    void saveSettings();
    QWidget* settingsWidget();
signals:
    void searchDone(QList<ScraperSearchResult>);
private slots:
    void searchFinished();
    void loadFinished();
    void posterFinished();
    void startNextPosterDownload();
    void posterSubFinished();
private:
    Movie *m_currentMovie;
    QNetworkAccessManager m_qnam;
    QNetworkReply *m_searchReply;
    QNetworkReply *m_loadReply;
    QNetworkReply *m_posterReply;
    QNetworkReply *m_posterSubReply;
    QQueue<QUrl> m_posterQueue;
    QNetworkAccessManager *qnam();
    QList<ScraperSearchResult> parseSearch(QString html);
    void parseAndAssignInfos(QString data, Movie *movie);
};

#endif // CINEFACTS_H
