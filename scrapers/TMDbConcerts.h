#ifndef TMDBCONCERTS_H
#define TMDBCONCERTS_H

#include <QComboBox>
#include <QLocale>
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
    ~TMDbConcerts() override = default;
    QString name() override;
    void search(QString searchStr) override;
    void loadData(QString id, Concert *concert, QList<int> infos) override;
    bool hasSettings() override;
    void loadSettings(QSettings &settings) override;
    void saveSettings(QSettings &settings) override;
    QList<int> scraperSupports() override;
    QWidget *settingsWidget() override;

signals:
    void searchDone(QList<ScraperSearchResult>) override;

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
    QLocale m_locale;
    QString m_language2;
    QString m_baseUrl;
    QList<int> m_scraperSupports;
    QWidget *m_widget;
    QComboBox *m_box;

    void setup();
    QString localeForTMDb() const;
    QString language() const;
    QString country() const;
    QNetworkAccessManager *qnam();
    QList<ScraperSearchResult> parseSearch(QString json, int *nextPage);
    void parseAndAssignInfos(QString json, Concert *concert, QList<int> infos);
};

#endif // TMDBCONCERTS_H
