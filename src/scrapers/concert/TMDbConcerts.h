#pragma once

#include "scrapers/concert/ConcertScraperInterface.h"

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
    explicit TMDbConcerts(QObject* parent = nullptr);
    static constexpr const char* scraperIdentifier = "TMDbConcerts";

    ~TMDbConcerts() override = default;
    QString name() const override;
    QString identifier() const override;
    void search(QString searchStr) override;
    void loadData(TmdbId id, Concert* concert, QVector<ConcertScraperInfos> infos) override;
    bool hasSettings() const override;
    void loadSettings(const ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;
    QVector<ConcertScraperInfos> scraperSupports() override;
    QWidget* settingsWidget() override;

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
    QVector<ConcertScraperInfos> m_scraperSupports;
    QWidget* m_widget;
    QComboBox* m_box;

    void setup();
    QString localeForTMDb() const;
    QString language() const;
    QString country() const;
    QNetworkAccessManager* qnam();
    QVector<ScraperSearchResult> parseSearch(QString json, int& nextPage);
    void parseAndAssignInfos(QString json, Concert* concert, QVector<ConcertScraperInfos> infos);
};
