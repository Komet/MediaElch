#ifndef TMDBCONCERTS_H
#define TMDBCONCERTS_H

#include <QComboBox>
#include <QObject>
#include <QWidget>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include "data/ConcertScraperInterface.h"

/**
 * @brief The TMDbConcerts class
 */
class TMDbConcerts : public ConcertScraperInterface
{
    Q_OBJECT
public:
    explicit TMDbConcerts(QObject *parent = nullptr);
    ~TMDbConcerts();
    QString name();
    void search(QString searchStr);
    void loadData(QString id, Concert *concert, QList<int> infos);
    bool hasSettings();
    void loadSettings(QSettings &settings);
    void saveSettings(QSettings &settings);
    QList<int> scraperSupports();
    QWidget *settingsWidget();

signals:
    void searchDone(QList<ScraperSearchResult>);

private slots:
    void searchFinished();
    void loadFinished();
    void loadTrailersFinished();
    void loadImagesFinished();
    void loadReleasesFinished();
    void setupFinished();

private:
    QString m_apiKey;
    QNetworkAccessManager m_qnam;
    QString m_language;
    QString m_language2;
    QString m_baseUrl;
    QList<int> m_scraperSupports;
    QWidget *m_widget;
    QComboBox *m_box;

    void setup();
    QNetworkAccessManager *qnam();
    QList<ScraperSearchResult> parseSearch(QString json, int *nextPage);
    void parseAndAssignInfos(QString json, Concert *concert, QList<int> infos);
};

#endif // TMDBCONCERTS_H
