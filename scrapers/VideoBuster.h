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

signals:
    void searchDone(QList<ScraperSearchResult>);

private slots:
    void searchFinished();
    void loadFinished();

private:
    QNetworkAccessManager m_qnam;
    QList<int> m_scraperSupports;

    QNetworkAccessManager *qnam();
    QList<ScraperSearchResult> parseSearch(QString html);
    void parseAndAssignInfos(QString html, Movie *movie, QList<int> infos);
    QString replaceEntities(const QString msg);
};

#endif // VIDEOBUSTER_H
