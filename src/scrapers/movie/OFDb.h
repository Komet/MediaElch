#pragma once

#include "scrapers/movie/MovieScraperInterface.h"

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

/**
 * @brief The OFDb class
 */
class OFDb : public MovieScraperInterface
{
    Q_OBJECT
public:
    explicit OFDb(QObject* parent = nullptr);
    static constexpr const char* scraperIdentifier = "ofdb";

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
    void searchFinished();
    void loadFinished();

private:
    QNetworkAccessManager m_qnam;
    QSet<MovieScraperInfos> m_scraperSupports;

    QNetworkAccessManager* qnam();
    QVector<ScraperSearchResult> parseSearch(QString xml, QString searchStr);
    void parseAndAssignInfos(QString data, Movie* movie, QSet<MovieScraperInfos> infos);
};
