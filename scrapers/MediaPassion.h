#ifndef MEDIAPASSION_H
#define MEDIAPASSION_H

#include <QLineEdit>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QObject>
#include <QPointer>
#include <QWidget>

#include "data/ScraperInterface.h"

class MediaPassion : public ScraperInterface
{
    Q_OBJECT
public:
    explicit MediaPassion(QObject *parent = 0);
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
    static QString apiKey();

signals:
    void searchDone(QList<ScraperSearchResult>);

private slots:
    void onSearchFinished();

private:
    QNetworkAccessManager m_qnam;
    QString m_baseUrl;
    QWidget *m_widget;
    QString m_username;
    QString m_password;
    QString m_usernameEnc;
    QString m_passwordEnc;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QList<int> m_scraperSupports;
    QList<int> m_scraperNativelySupports;

    QNetworkAccessManager *qnam();
};

#endif // MEDIAPASSION_H
