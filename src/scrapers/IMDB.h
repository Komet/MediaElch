#ifndef IMDB_H
#define IMDB_H

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "data/Movie.h"
#include "data/ScraperInterface.h"

class QCheckBox;

class IMDB : public ScraperInterface
{
    Q_OBJECT
public:
    explicit IMDB(QObject *parent = nullptr);
    QString name() override;
    QString identifier() override;
    void search(QString searchStr) override;
    void loadData(QMap<ScraperInterface *, QString> ids, Movie *movie, QList<MovieScraperInfos> infos) override;
    bool hasSettings() override;
    void loadSettings(QSettings &settings) override;
    void saveSettings(QSettings &settings) override;
    QList<MovieScraperInfos> scraperSupports() override;
    QList<MovieScraperInfos> scraperNativelySupports() override;
    std::vector<ScraperLanguage> supportedLanguages() override;
    QWidget *settingsWidget() override;
    bool isAdult() override;
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
