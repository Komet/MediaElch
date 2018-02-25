#ifndef OFDB_H
#define OFDB_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include "data/ScraperInterface.h"

/**
 * @brief The OFDb class
 */
class OFDb : public ScraperInterface
{
    Q_OBJECT
public:
    explicit OFDb(QObject *parent = nullptr);
    QString name();
    QString identifier();
    void search(QString searchStr);
    void loadData(QMap<ScraperInterface *, QString> ids, Movie *movie, QList<int> infos);
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
    QList<ScraperSearchResult> parseSearch(QString html, QString searchStr);
    void parseAndAssignInfos(QString data, Movie *movie, QList<int> infos);
};

#endif // OFDB_H
