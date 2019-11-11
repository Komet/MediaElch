#pragma once

#include "data/TmdbId.h"
#include "scrapers/movie/MovieScraperInterface.h"

#include <QComboBox>
#include <QLocale>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QPointer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

/**
 * @brief The TMDb class
 */
class TMDb : public MovieScraperInterface
{
    Q_OBJECT
public:
    explicit TMDb(QObject* parent = nullptr);
    static constexpr const char* scraperIdentifier = "TMDb";

    ~TMDb() override = default;
    QString name() const override;
    QString identifier() const override;
    void search(QString searchStr) override;
    void loadData(QMap<MovieScraperInterface*, QString> ids, Movie* movie, QVector<MovieScraperInfos> infos) override;
    bool hasSettings() const override;
    void loadSettings(const ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;
    QVector<MovieScraperInfos> scraperSupports() override;
    QVector<MovieScraperInfos> scraperNativelySupports() override;
    std::vector<ScraperLanguage> supportedLanguages() override;
    void changeLanguage(QString languageKey) override;
    QString defaultLanguageKey() override;
    QWidget* settingsWidget() override;
    static QVector<ScraperSearchResult> parseSearch(QString json, int* nextPage, int page);
    static QString apiKey();
    bool isAdult() const override;

private slots:
    void searchFinished();
    void loadFinished();
    void loadCollectionFinished();
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
    QVector<MovieScraperInfos> m_scraperSupports;
    QVector<MovieScraperInfos> m_scraperNativelySupports;
    QWidget* m_widget;
    QComboBox* m_box;

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
    QUrl getMovieSearchUrl(const QString& searchStr, const UrlParameterMap& parameters) const;
    QUrl
    getMovieUrl(QString movieId, ApiMovieDetails type, const UrlParameterMap& parameters = UrlParameterMap{}) const;
    QUrl getCollectionUrl(QString collectionId) const;

    void parseAndAssignInfos(QString json, Movie* movie, QVector<MovieScraperInfos> infos);
    /// Load the given collection (TMDb id) and store the content in the movie.
    void loadCollection(Movie* movie, const TmdbId& collectionTmdbId);
};
