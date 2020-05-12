#pragma once

#include "scrapers/movie/MovieScraperInterface.h"

#include <QObject>
#include <QWidget>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

/**
 * @brief The VideoBuster class
 */
class VideoBuster : public MovieScraperInterface
{
    Q_OBJECT
public:
    explicit VideoBuster(QObject* parent = nullptr);
    static constexpr const char* scraperIdentifier = "videobuster";

    QString name() const override;
    QString identifier() const override;
    void search(QString searchStr) override;
    void loadData(QHash<MovieScraperInterface*, QString> ids, Movie* movie, QSet<MovieScraperInfos> infos) override;
    bool hasSettings() const override;
    void loadSettings(const ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;
    QSet<MovieScraperInfos> scraperSupports() override;
    QSet<MovieScraperInfos> scraperNativelySupports() override;
    std::vector<ScraperLanguage> supportedLanguages() override;
    void changeLanguage(QString languageKey) override;
    QString defaultLanguageKey() override;
    QWidget* settingsWidget() override;
    bool isAdult() const override;

private slots:
    void searchFinished();
    void loadFinished();

private:
    QNetworkAccessManager m_qnam;
    QSet<MovieScraperInfos> m_scraperSupports;

    QNetworkAccessManager* qnam();
    QVector<ScraperSearchResult> parseSearch(QString html);
    void parseAndAssignInfos(QString html, Movie* movie, QSet<MovieScraperInfos> infos);
    QString replaceEntities(const QString msg);
};
