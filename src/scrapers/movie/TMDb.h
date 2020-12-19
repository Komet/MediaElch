#pragma once

#include "data/TmdbId.h"
#include "network/NetworkManager.h"
#include "scrapers/movie/MovieScraperInterface.h"

#include <QComboBox>
#include <QLocale>
#include <QMap>
#include <QMutex>
#include <QNetworkReply>
#include <QObject>
#include <QPointer>

namespace mediaelch {
namespace scraper {

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
    void loadData(QHash<MovieScraperInterface*, QString> ids, Movie* movie, QSet<MovieScraperInfo> infos) override;
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;
    QSet<MovieScraperInfo> scraperSupports() override;
    QSet<MovieScraperInfo> scraperNativelySupports() override;
    QVector<mediaelch::Locale> supportedLanguages() override;
    void changeLanguage(mediaelch::Locale locale) override;
    mediaelch::Locale defaultLanguage() override;
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
    mediaelch::network::NetworkManager m_network;
    mediaelch::Locale m_locale = mediaelch::Locale::English;
    QString m_baseUrl;
    QMutex m_mutex;
    QSet<MovieScraperInfo> m_scraperSupports;
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
