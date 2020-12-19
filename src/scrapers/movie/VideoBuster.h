#pragma once

#include "network/NetworkManager.h"
#include "scrapers/movie/MovieScraper.h"

#include <QNetworkReply>
#include <QObject>
#include <QWidget>


namespace mediaelch {
namespace scraper {

class VideoBuster : public MovieScraper
{
    Q_OBJECT
public:
    explicit VideoBuster(QObject* parent = nullptr);
    static constexpr const char* ID = "videobuster";

    QString name() const override;
    QString identifier() const override;
    void search(QString searchStr) override;
    void loadData(QHash<MovieScraper*, QString> ids, Movie* movie, QSet<MovieScraperInfo> infos) override;
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;
    QSet<MovieScraperInfo> scraperSupports() override;
    QSet<MovieScraperInfo> scraperNativelySupports() override;
    QVector<mediaelch::Locale> supportedLanguages() override;
    void changeLanguage(mediaelch::Locale locale) override;
    mediaelch::Locale defaultLanguage() override;
    QWidget* settingsWidget() override;
    bool isAdult() const override;

private slots:
    void searchFinished();
    void loadFinished();

private:
    mediaelch::network::NetworkManager m_network;
    QSet<MovieScraperInfo> m_scraperSupports;

    mediaelch::network::NetworkManager* network();
    QVector<ScraperSearchResult> parseSearch(QString html);
    void parseAndAssignInfos(const QString& html, Movie* movie, QSet<MovieScraperInfo> infos);
    QString replaceEntities(const QString msg);
};

} // namespace scraper
} // namespace mediaelch
