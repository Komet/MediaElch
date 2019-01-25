#ifndef TMDBCONCERTS_H
#define TMDBCONCERTS_H

#include "data/ConcertScraperInterface.h"

#include <QComboBox>
#include <QLocale>
#include <QObject>
#include <QWidget>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

/**
 * @brief The TMDbConcerts class
 */
class TMDbConcerts : public ConcertScraperInterface
{
    Q_OBJECT
public:
    explicit TMDbConcerts(QObject *parent = nullptr);
    ~TMDbConcerts() override = default;
    QString name() const override;
    QString identifier() const override;
    void search(QString searchStr) override;
    void loadData(TmdbId id, Concert *concert, QList<ConcertScraperInfos> infos) override;
    bool hasSettings() const override;
    void loadSettings(const ScraperSettings &settings) override;
    void saveSettings(ScraperSettings &settings) override;
    QList<ConcertScraperInfos> scraperSupports() override;
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
    QList<ConcertScraperInfos> m_scraperSupports;
    QWidget *m_widget;
    QComboBox *m_box;

    void setup();
    QString localeForTMDb() const;
    QString language() const;
    QString country() const;
    QNetworkAccessManager *qnam();
    QList<ScraperSearchResult> parseSearch(QString json, int &nextPage);
    void parseAndAssignInfos(QString json, Concert *concert, QList<ConcertScraperInfos> infos);
};

#endif // TMDBCONCERTS_H
