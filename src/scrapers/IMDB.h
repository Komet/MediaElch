#ifndef IMDB_H
#define IMDB_H

#include "data/Movie.h"
#include "data/MovieScraperInterface.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

class QCheckBox;

class IMDB : public MovieScraperInterface
{
    Q_OBJECT
public:
    explicit IMDB(QObject *parent = nullptr);
    QString name() const override;
    QString identifier() const override;
    void search(QString searchStr) override;
    void loadData(QMap<MovieScraperInterface *, QString> ids, Movie *movie, QList<MovieScraperInfos> infos) override;
    bool hasSettings() const override;
    void loadSettings(QSettings &settings) override;
    void saveSettings(QSettings &settings) override;
    QList<MovieScraperInfos> scraperSupports() override;
    QList<MovieScraperInfos> scraperNativelySupports() override;
    std::vector<ScraperLanguage> supportedLanguages() override;
    void changeLanguage(QString languageKey) override;
    QString defaultLanguageKey() override;
    QWidget *settingsWidget() override;
    bool isAdult() const override;
    void parseAndAssignInfos(QString html, Movie *movie, QList<MovieScraperInfos> infos);

signals:
    void searchDone(QList<ScraperSearchResult>) override;

private slots:
    void onSearchFinished();
    void onSearchIdFinished();
    void onLoadFinished();
    void onPosterLoadFinished();
    void onTagsFinished();

private:
    QWidget *m_settingsWidget;
    QCheckBox *m_loadAllTagsWidget;

    bool m_loadAllTags;
    QNetworkAccessManager m_qnam;
    QList<MovieScraperInfos> m_scraperSupports;

    QList<ScraperSearchResult> parseSearch(QString html);
    void parseAndAssignPoster(QString html, QString posterId, Movie *movie, QList<MovieScraperInfos> infos);
    QUrl parsePosters(QString html);
    void parseAndAssignTags(const QString &html, Movie &movie);
};

#endif // IMDB_H
