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
    void backdropFinished();
    void startNextPosterDownload();
    void posterSubFinished();
    void startNextBackdropDownload();
    void backdropSubFinished();
private:
    Movie *m_currentMovie;
    QNetworkAccessManager m_qnam;
    QNetworkReply *m_searchReply;
    QNetworkReply *m_loadReply;
    QNetworkReply *m_posterReply;
    QNetworkReply *m_backdropReply;
    QNetworkReply *m_posterSubReply;
    QNetworkReply *m_backdropSubReply;
    QQueue<QUrl> m_posterQueue;
    QQueue<QUrl> m_backdropQueue;
    QNetworkAccessManager *qnam();
    QList<ScraperSearchResult> parseSearch(QString html);
    QUrl m_backdropUrl;
    void parseAndAssignInfos(QString data, Movie *movie);
};

#endif // CINEFACTS_H
