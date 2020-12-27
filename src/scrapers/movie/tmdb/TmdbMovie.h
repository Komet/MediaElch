#pragma once

#include "data/TmdbId.h"
#include "network/NetworkManager.h"
#include "scrapers/movie/MovieScraper.h"

#include <QComboBox>
#include <QLocale>
#include <QMap>
#include <QMutex>
#include <QNetworkReply>
#include <QObject>
#include <QPointer>

namespace mediaelch {
namespace scraper {

class TmdbMovie : public MovieScraper
{
    Q_OBJECT
public:
    explicit TmdbMovie(QObject* parent = nullptr);
    ~TmdbMovie() override = default;
    static constexpr const char* ID = "TMDb";

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

public:
    void search(QString searchStr) override;
    void loadData(QHash<MovieScraper*, QString> ids, Movie* movie, QSet<MovieScraperInfo> infos) override;
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;
    QSet<MovieScraperInfo> scraperNativelySupports() override;
    void changeLanguage(mediaelch::Locale locale) override;
    QWidget* settingsWidget() override;
    static QVector<ScraperSearchResult> parseSearch(QString json, int* nextPage, int page);
    static QString apiKey();

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
    ScraperMeta m_meta;
    mediaelch::network::NetworkManager m_network;
    QString m_baseUrl;
    QMutex m_mutex;
    QSet<MovieScraperInfo> m_scraperNativelySupports;
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

    void parseAndAssignInfos(QString json, Movie* movie, QSet<MovieScraperInfo> infos);
    /// Load the given collection (TMDb id) and store the content in the movie.
    void loadCollection(Movie* movie, const TmdbId& collectionTmdbId);
};

} // namespace scraper
} // namespace mediaelch
