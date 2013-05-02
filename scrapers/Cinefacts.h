#ifndef CINEFACTS_H
#define CINEFACTS_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QQueue>
#include <QObject>

#include "data/ScraperInterface.h"

/**
 * @brief The Cinefacts class
 */
class Cinefacts : public ScraperInterface
{
    Q_OBJECT
public:
    explicit Cinefacts(QObject *parent = 0);
    QString name();
    QString identifier();
    void search(QString searchStr);
    void loadData(QString id, Movie *movie, QList<int> infos);
    bool hasSettings();
    void loadSettings(QSettings &settings);
    void saveSettings(QSettings &settings);
    QList<int> scraperSupports();
    QList<int> scraperNativelySupports();
    QMap<QString, QString> languages();
    QString language();
    void setLanguage(QString language);

signals:
    void searchDone(QList<ScraperSearchResult>);

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
