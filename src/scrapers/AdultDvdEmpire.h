#ifndef ADULTDVDEMPIRE_H
#define ADULTDVDEMPIRE_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QWidget>

#include "data/ScraperInterface.h"

class AdultDvdEmpire : public ScraperInterface
{
    Q_OBJECT
public:
    explicit AdultDvdEmpire(QObject *parent = nullptr);
    QString name() override;
    QString identifier() override;
    void search(QString searchStr) override;
    void loadData(QMap<ScraperInterface *, QString> ids, Movie *movie, QList<MovieScraperInfos> infos) override;
    bool hasSettings() override;
    void loadSettings(QSettings &settings) override;
    void saveSettings(QSettings &settings) override;
    QList<MovieScraperInfos> scraperSupports() override;
    QList<MovieScraperInfos> scraperNativelySupports() override;
    QWidget *settingsWidget() override;
    bool isAdult() override;

signals:
    void searchDone(QList<ScraperSearchResult>) override;

private slots:
    void onSearchFinished();
    void onLoadFinished();

private:
    QNetworkAccessManager m_qnam;
    QList<MovieScraperInfos> m_scraperSupports;

    QNetworkAccessManager *qnam();
    QList<ScraperSearchResult> parseSearch(QString html);
    void parseAndAssignInfos(QString html, Movie *movie, QList<MovieScraperInfos> infos);
};

#endif // ADULTDVDEMPIRE_H
