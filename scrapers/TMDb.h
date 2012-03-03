#ifndef TMDB_H
#define TMDB_H

#include <QComboBox>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QObject>
#include <QWidget>

#include "data/ScraperInterface.h"

class TMDb : public ScraperInterface
{
    Q_OBJECT
public:
    explicit TMDb(QObject *parent = 0);
    ~TMDb();
    QString name();
    void search(QString searchStr);
    void loadData(QString id, Movie *movie);
    bool hasSettings();
    QWidget *settingsWidget();
    void loadSettings();
    void saveSettings();

signals:
    void searchDone(QList<ScraperSearchResult>);

private slots:
    void searchFinished();
    void loadFinished();

private:
    QString m_apiKey;
    QNetworkAccessManager m_qnam;
    QNetworkReply *m_searchReply;
    QNetworkReply *m_loadReply;
    Movie *m_currentMovie;
    QComboBox *m_settingsLanguageCombo;
    QString m_language;
    QNetworkAccessManager *qnam();
    QList<ScraperSearchResult> parseSearch(QString json);
    void parseAndAssignInfos(QString json, Movie *movie);
};

#endif // TMDB_H
