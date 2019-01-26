#pragma once

#include "data/MovieScraperInterface.h"

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
    explicit OFDb(QObject *parent = nullptr);
    QString name() const override;
    QString identifier() const override;
    void search(QString searchStr) override;
    void loadData(QMap<MovieScraperInterface *, QString> ids, Movie *movie, QList<MovieScraperInfos> infos) override;
    bool hasSettings() const override;
    void loadSettings(const ScraperSettings &settings) override;
    void saveSettings(ScraperSettings &settings) override;
    QList<MovieScraperInfos> scraperSupports() override;
    QList<MovieScraperInfos> scraperNativelySupports() override;
    std::vector<ScraperLanguage> supportedLanguages() override;
    void changeLanguage(QString languageKey) override;
    QString defaultLanguageKey() override;
    QWidget *settingsWidget() override;
    bool isAdult() const override;

signals:
    void searchDone(QList<ScraperSearchResult>) override;

private slots:
    void searchFinished();
    void loadFinished();

private:
    QNetworkAccessManager m_qnam;
    QList<MovieScraperInfos> m_scraperSupports;

    QNetworkAccessManager *qnam();
    QList<ScraperSearchResult> parseSearch(QString html, QString searchStr);
    void parseAndAssignInfos(QString data, Movie *movie, QList<MovieScraperInfos> infos);
};
