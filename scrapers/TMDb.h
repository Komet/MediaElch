#ifndef TMDB_H
#define TMDB_H

#include <QComboBox>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QObject>
#include <QPointer>

#include "data/ScraperInterface.h"

/**
 * @brief The TMDb class
 */
class TMDb : public ScraperInterface
{
    Q_OBJECT
public:
    explicit TMDb(QObject *parent = 0);
    ~TMDb();
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
    static QList<ScraperSearchResult> parseSearch(QString json, int *nextPage, int page);
    static QString apiKey();
    bool isAdult();

signals:
    void searchDone(QList<ScraperSearchResult>);

private slots:
    void searchFinished();
    void loadFinished();
    void loadCastsFinished();
    void loadTrailersFinished();
    void loadImagesFinished();
    void loadReleasesFinished();
    void setupFinished();

private:
    QNetworkAccessManager m_qnam;
    QString m_language;
    QString m_language2;
    QString m_baseUrl;
    QMutex m_mutex;
    QList<int> m_scraperSupports;
    QList<int> m_scraperNativelySupports;
    QWidget *m_widget;
    QComboBox *m_box;

    QNetworkAccessManager *qnam();
    void setup();
    void parseAndAssignInfos(QString json, Movie *movie, QList<int> infos);
};

#endif // TMDB_H
