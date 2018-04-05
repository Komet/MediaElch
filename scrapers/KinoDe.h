#ifndef CINEFACTS_H
#define CINEFACTS_H

#include <QObject>
#include <QQueue>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include "data/ScraperInterface.h"

/**
 * @brief The Kino.de class
 */
class KinoDe : public ScraperInterface
{
    Q_OBJECT
public:
    explicit KinoDe(QObject *parent = nullptr);
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

private:
    QList<ScraperSearchResult> parseSearch(const QString &html);
    void parseAndAssignInfos(const QString &html, Movie &movie, const QList<int> &infos);
    void parseAndAssignActors(const QString &html, Movie &movie, const QList<int> &infos);
    void parseAndAssignImages(const QString &html, Movie &movie, const QList<int> &infos);
    void parsePoster(const QString &html, Movie &movie);
    void parseBackdrops(const QString &html, Movie &movie);

    QNetworkAccessManager m_qnam;
    QList<int> m_scraperSupports;
};

#endif // CINEFACTS_H
