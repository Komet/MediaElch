#ifndef MEDIAPASSION_H
#define MEDIAPASSION_H

#include <QComboBox>
#include <QLineEdit>
#include <QObject>
#include <QPointer>
#include <QWidget>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include "data/ScraperInterface.h"

class MediaPassion : public ScraperInterface
{
    Q_OBJECT
public:
    explicit MediaPassion(QObject *parent = nullptr);
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
    static QString apiKey();
    bool isAdult() override;

signals:
    void searchDone(QList<ScraperSearchResult>) override;

private slots:
    void onSearchFinished();
    void onLoadFinished();

private:
    QNetworkAccessManager m_qnam;
    QString m_baseUrl;
    QWidget *m_widget;
    QString m_username;
    QString m_password;
    QString m_usernameEnc;
    QString m_passwordEnc;
    QString m_ratingType;
    QString m_certificationNation;
    QString m_language;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QComboBox *m_ratingCombo;
    QComboBox *m_certificationCombo;
    QComboBox *m_languageCombo;
    QList<int> m_scraperSupports;
    QList<int> m_scraperNativelySupports;
    static QMap<QUrl, QString> m_contentCache;

    QNetworkAccessManager *qnam();
    QList<ScraperSearchResult> parseSearch(QString xml);
    bool checkUserAndPass();
    bool hasError(QString xml, QString &errorMsg);
    void parseAndAssignInfos(QString data, Movie *movie, QList<int> infos);
};

#endif // MEDIAPASSION_H
