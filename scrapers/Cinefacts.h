#ifndef CINEFACTS_H
#define CINEFACTS_H

#include <QObject>
#include <QQueue>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include "data/ScraperInterface.h"

/**
 * @brief The Cinefacts class
 */
class Cinefacts : public ScraperInterface
{
    Q_OBJECT
public:
    explicit Cinefacts(QObject *parent = nullptr);
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
    void searchFinished();
    void loadFinished();
    void actorsFinished();
    void imagesFinished();
    void posterFinished();
    void backdropFinished();

private:
    QNetworkAccessManager m_qnam;
    QList<int> m_scraperSupports;

    QNetworkAccessManager *qnam();
    QList<ScraperSearchResult> parseSearch(QString html);
    void parseAndAssignInfos(QString data, Movie *movie, QList<int> infos);
    void parseAndAssignActors(QString data, Movie *movie, QList<int> infos);
    void parseImages(QString data, QStringList &posters, QStringList &backgrounds);
};

#endif // CINEFACTS_H
