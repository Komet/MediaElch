#pragma once

#include "movies/Movie.h"
#include "scrapers/movie/MovieScraperInterface.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

class QCheckBox;

class IMDB : public MovieScraperInterface
{
    Q_OBJECT
public:
    explicit IMDB(QObject* parent = nullptr);
    static constexpr const char* scraperIdentifier = "IMDb";

    QString name() const override;
    QString identifier() const override;
    void search(QString searchStr) override;
    void loadData(QMap<MovieScraperInterface*, QString> ids, Movie* movie, QVector<MovieScraperInfos> infos) override;
    bool hasSettings() const override;
    void loadSettings(const ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;
    QVector<MovieScraperInfos> scraperSupports() override;
    QVector<MovieScraperInfos> scraperNativelySupports() override;
    std::vector<ScraperLanguage> supportedLanguages() override;
    void changeLanguage(QString languageKey) override;
    QString defaultLanguageKey() override;
    QWidget* settingsWidget() override;
    bool isAdult() const override;
    void parseAndAssignInfos(const QString& html, Movie* movie, QVector<MovieScraperInfos> infos);

private slots:
    void onSearchFinished();
    void onSearchIdFinished();
    void onLoadFinished();
    void onPosterLoadFinished();
    void onTagsFinished();

private:
    QWidget* m_settingsWidget;
    QCheckBox* m_loadAllTagsWidget;

    bool m_loadAllTags = false;
    QNetworkAccessManager m_qnam;
    QVector<MovieScraperInfos> m_scraperSupports;

    QVector<ScraperSearchResult> parseSearch(QString html);
    void parseAndAssignPoster(const QString& html, QString posterId, Movie* movie);
    QUrl parsePosters(QString html);
    void parseAndAssignTags(const QString& html, Movie& movie);
};
