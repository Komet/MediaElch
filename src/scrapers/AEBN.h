#ifndef AEBN_H
#define AEBN_H

#include "data/MovieScraperInterface.h"

#include <QComboBox>
#include <QMap>
#include <QNetworkAccessManager>
#include <QObject>
#include <QWidget>

class AEBN : public MovieScraperInterface
{
    Q_OBJECT
public:
    explicit AEBN(QObject *parent = nullptr);
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

signals:
    void searchDone(QList<ScraperSearchResult>) override;

private slots:
    void onSearchFinished();
    void onLoadFinished();
    void onActorLoadFinished();

private:
    QNetworkAccessManager m_qnam;
    QList<MovieScraperInfos> m_scraperSupports;
    QString m_language;
    QString m_genreId;
    QWidget *m_widget;
    QComboBox *m_box;
    QComboBox *m_genreBox;

    QList<ScraperSearchResult> parseSearch(QString html);
    void parseAndAssignInfos(QString html, Movie *movie, QList<MovieScraperInfos> infos, QStringList &actorIds);
    void downloadActors(Movie *movie, QStringList actorIds);
    void parseAndAssignActor(QString html, Movie *movie, QString id);
};

#endif // AEBN_H
