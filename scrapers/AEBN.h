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
    explicit AEBN(QObject *parent = nullptr);
    QString name() override;
    QString identifier() override;
    void search(QString searchStr) override;
    void loadData(QMap<ScraperInterface *, QString> ids, Movie *movie, QList<int> infos) override;
    bool hasSettings() override;
    void loadSettings(QSettings &settings) override;
    void saveSettings(QSettings &settings) override;
    QList<int> scraperSupports() override;
    QList<int> scraperNativelySupports() override;
    QWidget *settingsWidget() override;
    bool isAdult() override;

signals:
    void searchDone(QList<ScraperSearchResult>) override;

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
