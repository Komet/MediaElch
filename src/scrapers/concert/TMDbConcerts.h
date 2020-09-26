#pragma once

#include "network/NetworkManager.h"
#include "scrapers/concert/ConcertScraperInterface.h"
#include "settings/ScraperSettings.h"

#include <QComboBox>
#include <QLocale>
#include <QNetworkReply>
#include <QObject>
#include <QWidget>

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
    void loadData(TmdbId id, Concert* concert, QSet<ConcertScraperInfo> infos) override;
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;
    QSet<ConcertScraperInfo> scraperSupports() override;
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
    mediaelch::network::NetworkManager m_network;
    QLocale m_locale;
    QString m_language2;
    QString m_baseUrl;
    QSet<ConcertScraperInfo> m_scraperSupports;
    QWidget* m_widget;
    QComboBox* m_box;

    void setup();
    QString localeForTMDb() const;
    QString language() const;
    QString country() const;
    mediaelch::network::NetworkManager* network();
    QVector<ScraperSearchResult> parseSearch(QString json, int& nextPage);
    void parseAndAssignInfos(QString json, Concert* concert, QSet<ConcertScraperInfo> infos);
};
