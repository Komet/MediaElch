#pragma once

#include "network/NetworkManager.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/aebn/AebnApi.h"

#include <QComboBox>
#include <QMap>
#include <QObject>
#include <QWidget>


namespace mediaelch {
namespace scraper {

class AEBN : public MovieScraper
{
    Q_OBJECT
public:
    explicit AEBN(QObject* parent = nullptr);
    static constexpr const char* ID = "aebn";

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

public:
    void search(QString searchStr) override;
    void loadData(QHash<MovieScraper*, QString> ids, Movie* movie, QSet<MovieScraperInfo> infos) override;
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;

    QSet<MovieScraperInfo> scraperNativelySupports() override;

    void changeLanguage(mediaelch::Locale locale) override;
    QWidget* settingsWidget() override;

private:
    ScraperMeta m_meta;
    AebnApi m_api;

    mediaelch::network::NetworkManager m_network;
    mediaelch::Locale m_language;
    QString m_genreId;
    QWidget* m_widget;
    QComboBox* m_box;
    QComboBox* m_genreBox;

    QVector<ScraperSearchResult> parseSearch(QString html);
    void parseAndAssignInfos(QString html, Movie* movie, QSet<MovieScraperInfo> infos, QStringList& actorIds);
    void downloadActors(Movie* movie, QStringList actorIds);
    void parseAndAssignActor(QString html, Movie* movie, QString id);
};

} // namespace scraper
} // namespace mediaelch
