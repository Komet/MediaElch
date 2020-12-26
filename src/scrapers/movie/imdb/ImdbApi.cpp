#include "ImdbApi.h"

#include <QUrl>
#include <QUrlQuery>

namespace mediaelch {
namespace scraper {

ImdbApi::ImdbApi(QObject* parent) : QObject(parent)
{
}

void ImdbApi::sendGetRequest(const Locale& locale, const QUrl& url, ImdbApi::ApiCallback callback)
{
    if (m_cache.hasValidElement(url, locale)) {
        // Do not immediately run the callback because classes higher up may
        // set up a Qt connection while the network request is running.
        QTimer::singleShot(0, [cb = std::move(callback), element = m_cache.getElement(url, locale)]() {
            // should not result in a parse error because the cache element is
            // only stored if no error occured at all.
            cb(QJsonDocument::fromJson(element.toUtf8()), {});
        });
        return;
    }

    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);
    QNetworkReply* reply = m_network.getWithWatcher(request);

    connect(reply, &QNetworkReply::finished, [reply, cb = std::move(callback), locale, this]() {
        auto dls = makeDeleteLaterScope(reply);

        QString data;
        if (reply->error() == QNetworkReply::NoError) {
            data = QString::fromUtf8(reply->readAll());

        } else {
            qWarning() << "[ImdbApi] Network Error:" << reply->errorString() << "for URL" << reply->url();
        }

        QJsonParseError parseError{};
        QJsonDocument json;
        if (!data.isEmpty()) {
            json = QJsonDocument::fromJson(data.toUtf8(), &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                m_cache.addElement(reply->url(), locale, data);
            }
        }

        ScraperError error = makeScraperError(data, *reply, parseError);
        cb(json, error);
    });
}

void ImdbApi::searchForMovie(const Locale& locale,
    const QString& query,
    bool includeAdult,
    ImdbApi::ApiCallback callback)
{
    sendGetRequest(locale, makeMovieSearchUrl(query, locale, includeAdult), std::move(callback));
}

QUrl ImdbApi::makeApiUrl(const QString& suffix, const Locale& locale, QUrlQuery query) const
{
    return QStringLiteral("https://api.videobuster.org/%1?%2").arg(suffix, query.toString());
}

QUrl ImdbApi::makeMovieSearchUrl(const QString& searchStr, const Locale& locale) const
{
    QUrlQuery queries;
    queries.addQueryItem("page", "1"); // Only query first page as of now.
    queries.addQueryItem("query", searchStr);
    return makeApiUrl("/search/tv", locale, queries);
}

} // namespace scraper
} // namespace mediaelch
