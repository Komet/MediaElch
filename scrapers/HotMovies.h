#ifndef HOTMOVIES_H
#define HOTMOVIES_H

#include <QComboBox>
#include <QNetworkAccessManager>
#include <QObject>
#include <QWidget>
#include "data/ScraperInterface.h"

class HotMovies : public ScraperInterface
{
    Q_OBJECT
public:
    explicit HotMovies(QObject *parent = 0);
    QString name();
    QString identifier();
    void search(QString searchStr);
    void loadData(QMap<ScraperInterface*, QString> ids, Movie *movie, QList<int> infos);
    bool hasSettings();
    void loadSettings(QSettings &settings);
    void saveSettings(QSettings &settings);
    QList<int> scraperSupports();
    QList<int> scraperNativelySupports();
    QWidget *settingsWidget();
    bool isAdult();

signals:
    void searchDone(QList<ScraperSearchResult>);

private slots:
    void onSearchFinished();
    void onLoadFinished();

private:
    QNetworkAccessManager m_qnam;
    QList<int> m_scraperSupports;

    QNetworkAccessManager *qnam();
    QList<ScraperSearchResult> parseSearch(QString html);
    void parseAndAssignInfos(QString html, Movie *movie, QList<int> infos);
};

#endif // HOTMOVIES_H
