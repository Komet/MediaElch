#pragma once

#include "network/NetworkManager.h"
#include "scrapers/concert/ConcertScraperInterface.h"
#include "settings/ScraperSettings.h"

#include <QComboBox>
#include <QLocale>
#include <QNetworkReply>
#include <QObject>
#include <QWidget>

namespace mediaelch {
namespace scraper {

class TMDbConcerts : public ConcertScraperInterface
{
    Q_OBJECT
public:
    explicit TMDbConcerts(QObject* parent = nullptr);
    ~TMDbConcerts() override = default;

    static constexpr const char* ID = "TMDbConcerts";

    const ScraperMeta& meta() const override;

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
    ScraperMeta m_meta;

    QString m_apiKey;
    mediaelch::network::NetworkManager m_network;
    QLocale m_locale;
    QString m_language2;
    QString m_baseUrl;
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

} // namespace scraper
} // namespace mediaelch
