#pragma once

#include "data/TmdbId.h"
#include "scrapers/Locale.h"
#include "scrapers/ScraperInterface.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QVector>

namespace mediaelch {
namespace scraper {

class TmdbApi : public QObject
{
    Q_OBJECT

public:
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
        Locale,
        YEAR,
        PAGE,
        INCLUDE_ADULT
    };
    using UrlParameterMap = QMap<ApiUrlParameter, QString>;

public:
    static QVector<Locale> supportedLanguages();

public:
    TmdbApi(QString apiKey) : m_apiKey{std::move(apiKey)} {}

    const QString& key() const { return m_apiKey; }
    const QString& imageBaseUrl() const { return m_imageBaseUrl; }

    void setImageBaseUrl(const QString& baseUrl) { m_imageBaseUrl = baseUrl; }

    QUrl movieSearchUrl(const QString& searchStr, const UrlParameterMap& parameters) const;
    QUrl movieUrl(QString movieId, ApiMovieDetails type, const UrlParameterMap& parameters = UrlParameterMap{}) const;
    QUrl collectionUrl(QString collectionId) const;

    QNetworkReply* sendGetRequest(const QUrl& url);
    QNetworkAccessManager& network() { return m_qnam; }

private:
    QString m_apiKey;
    QString m_imageBaseUrl;
    QNetworkAccessManager m_qnam;
};

} // namespace scraper
} // namespace mediaelch
