#ifndef TMDB_H
#define TMDB_H

#include <QComboBox>
#include <QLocale>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include "data/ScraperInterface.h"

/**
 * @brief The TMDb class
 */
class TMDb : public ScraperInterface
{
    Q_OBJECT
public:
    explicit TMDb(QObject *parent = nullptr);
    ~TMDb() override = default;
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
    static QList<ScraperSearchResult> parseSearch(QString json, int *nextPage, int page);
    static QString apiKey();
    bool isAdult() override;

signals:
    void searchDone(QList<ScraperSearchResult>) override;

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
    QLocale m_locale;
    QString m_baseUrl;
    QMutex m_mutex;
    QList<int> m_scraperSupports;
    QList<int> m_scraperNativelySupports;
    QWidget *m_widget;
    QComboBox *m_box;

    enum class ApiMovieDetails
    {
        INFOS,
        IMAGES,
        CASTS,
        TRAILERS,
        RELEASES
    };
    enum class ApiUrlParameter
    {
        YEAR,
        PAGE,
        INCLUDE_ADULT
    };
    using UrlParameterMap = QMap<ApiUrlParameter, QString>;

    void setup();
    QString localeForTMDb() const;
    QString language() const;
    QString country() const;
    QString apiUrlParameterString(ApiUrlParameter parameter) const;
    QUrl getMovieSearchUrl(const QString &searchStr, const UrlParameterMap &parameters) const;
    QUrl getMovieUrl(const QString &title,
        ApiMovieDetails type,
        const UrlParameterMap &parameters = UrlParameterMap{}) const;
    void parseAndAssignInfos(QString json, Movie *movie, QList<int> infos);
};

#endif // TMDB_H
