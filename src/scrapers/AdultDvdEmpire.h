#pragma once

#include "data/MovieScraperInterface.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QWidget>

class AdultDvdEmpire : public MovieScraperInterface
{
    Q_OBJECT
public:
    explicit AdultDvdEmpire(QObject *parent = nullptr);
    QString name() const override;
    QString identifier() const override;
    void search(QString searchStr) override;
    void loadData(QMap<MovieScraperInterface *, QString> ids, Movie *movie, QVector<MovieScraperInfos> infos) override;
    bool hasSettings() const override;
    void loadSettings(const ScraperSettings &settings) override;
    void saveSettings(ScraperSettings &settings) override;
    QVector<MovieScraperInfos> scraperSupports() override;
    QVector<MovieScraperInfos> scraperNativelySupports() override;
    std::vector<ScraperLanguage> supportedLanguages() override;
    void changeLanguage(QString languageKey) override;
    QString defaultLanguageKey() override;
    QWidget *settingsWidget() override;
    bool isAdult() const override;

signals:
    void searchDone(QVector<ScraperSearchResult>) override;

private slots:
    void onSearchFinished();
    void onLoadFinished();

private:
    QNetworkAccessManager m_qnam;
    QVector<MovieScraperInfos> m_scraperSupports;

    QNetworkAccessManager *qnam();
    QVector<ScraperSearchResult> parseSearch(QString html);
    void parseAndAssignInfos(QString html, Movie *movie, QVector<MovieScraperInfos> infos);
};
