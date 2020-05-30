#pragma once

#include "scrapers/movie/MovieScraperInterface.h"

#include <QNetworkAccessManager>
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
    void loadData(QHash<MovieScraperInterface*, QString> ids, Movie* movie, QSet<MovieScraperInfos> infos) override;
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;
    QSet<MovieScraperInfos> scraperSupports() override;
    QSet<MovieScraperInfos> scraperNativelySupports() override;
    std::vector<ScraperLanguage> supportedLanguages() override;
    void changeLanguage(QString languageKey) override;
    QString defaultLanguageKey() override;
    QWidget* settingsWidget() override;
    bool isAdult() const override;

private slots:
    void onSearchFinished();
    void onLoadFinished();

private:
    QNetworkAccessManager m_qnam;
    QSet<MovieScraperInfos> m_scraperSupports;

    QNetworkAccessManager* qnam();
    QVector<ScraperSearchResult> parseSearch(QString html);
    void parseAndAssignInfos(QString html, Movie* movie, QSet<MovieScraperInfos> infos);
};
