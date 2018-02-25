#ifndef AEBN_H
#define AEBN_H

#include <QComboBox>
#include <QNetworkAccessManager>
#include <QObject>
#include <QWidget>

#include "data/ScraperInterface.h"

class AEBN : public ScraperInterface
{
    Q_OBJECT
public:
    explicit AEBN(QObject *parent = 0);
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
    void onActorLoadFinished();

private:
    QNetworkAccessManager m_qnam;
    QList<int> m_scraperSupports;
    QString m_language;
    QWidget *m_widget;
    QComboBox *m_box;

    QNetworkAccessManager *qnam();
    QList<ScraperSearchResult> parseSearch(QString html);
    void parseAndAssignInfos(QString html, Movie *movie, QList<int> infos, QStringList &actorIds);
    void downloadActors(Movie *movie, QStringList actorIds);
    void parseAndAssignActor(QString html, Movie *movie, QString id);
};

#endif // AEBN_H
