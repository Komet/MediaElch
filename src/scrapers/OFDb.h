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
    QString name() override;
    QString identifier() override;
    void search(QString searchStr) override;
    void loadData(QMap<ScraperInterface *, QString> ids, Movie *movie, QList<MovieScraperInfos> infos) override;
    bool hasSettings() override;
    void loadSettings(QSettings &settings) override;
    void saveSettings(QSettings &settings) override;
    QList<MovieScraperInfos> scraperSupports() override;
    QList<MovieScraperInfos> scraperNativelySupports() override;
    QWidget *settingsWidget() override;
    bool isAdult() override;

signals:
    void searchDone(QList<ScraperSearchResult>) override;

private slots:
    void searchFinished();
    void loadFinished();

private:
    QNetworkAccessManager m_qnam;
    QList<MovieScraperInfos> m_scraperSupports;

    QNetworkAccessManager *qnam();
    QList<ScraperSearchResult> parseSearch(QString html, QString searchStr);
    void parseAndAssignInfos(QString data, Movie *movie, QList<MovieScraperInfos> infos);
};

#endif // OFDB_H
