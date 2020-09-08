#pragma once

#include "network/NetworkManager.h"
#include "scrapers/movie/MovieScraperInterface.h"

#include <QObject>
#include <QWidget>

class AdultDvdEmpire : public MovieScraperInterface
{
    Q_OBJECT
public:
    explicit AdultDvdEmpire(QObject* parent = nullptr);
    static constexpr const char* scraperIdentifier = "adult-dvd-empire";

    QString name() const override;
    QString identifier() const override;
    void search(QString searchStr) override;
    void loadData(QHash<MovieScraperInterface*, QString> ids, Movie* movie, QSet<MovieScraperInfo> infos) override;
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
    void onSearchFinished();
    void onLoadFinished();

private:
    mediaelch::network::NetworkManager m_network;
    QSet<MovieScraperInfo> m_scraperSupports;

    mediaelch::network::NetworkManager* network();
    QVector<ScraperSearchResult> parseSearch(QString html);
    void parseAndAssignInfos(QString html, Movie* movie, QSet<MovieScraperInfo> infos);
};
