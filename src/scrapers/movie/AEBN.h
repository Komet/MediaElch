#pragma once

#include "scrapers/movie/MovieScraperInterface.h"

#include <QComboBox>
#include <QMap>
#include <QNetworkAccessManager>
#include <QObject>
#include <QWidget>

class AEBN : public MovieScraperInterface
{
    Q_OBJECT
public:
    explicit AEBN(QObject* parent = nullptr);
    static constexpr const char* scraperIdentifier = "aebn";

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

private slots:
    void onSearchFinished();
    void onLoadFinished();
    void onActorLoadFinished();

private:
    QNetworkAccessManager m_qnam;
    QVector<MovieScraperInfos> m_scraperSupports;
    QString m_language;
    QString m_genreId;
    QWidget* m_widget;
    QComboBox* m_box;
    QComboBox* m_genreBox;

    QVector<ScraperSearchResult> parseSearch(QString html);
    void parseAndAssignInfos(QString html, Movie* movie, QVector<MovieScraperInfos> infos, QStringList& actorIds);
    void downloadActors(Movie* movie, QStringList actorIds);
    void parseAndAssignActor(QString html, Movie* movie, QString id);
};
