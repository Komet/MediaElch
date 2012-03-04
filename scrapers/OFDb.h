#ifndef OFDB_H
#define OFDB_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QObject>
#include <QWidget>

#include "data/ScraperInterface.h"

class OFDb : public ScraperInterface
{
    Q_OBJECT
public:
    explicit OFDb(QObject *parent = 0);
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
private:
    Movie *m_currentMovie;
    QNetworkAccessManager m_qnam;
    QNetworkReply *m_searchReply;
    QNetworkReply *m_loadReply;
    int m_httpNotFoundCounter;
    QString m_currentSearchString;
    QString m_currentLoadId;
    QNetworkAccessManager *qnam();
    QList<ScraperSearchResult> parseSearch(QString html);
    void parseAndAssignInfos(QString data, Movie *movie);
};

#endif // OFDB_H
