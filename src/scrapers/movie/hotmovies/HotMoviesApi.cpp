#include "HotMoviesApi.h"

#include "log/Log.h"
#include "network/NetworkRequest.h"
#include "utils/Meta.h"

#include <QList>
#include <QNetworkCookie>

namespace {

constexpr int TOKEN_RESET_TIMEOUT_SECONDS = 10 * 60; // 10min

}

namespace mediaelch {
namespace scraper {

HotMoviesApi::HotMoviesApi(QObject* parent) : QObject(parent)
{
    m_tokenResetTimer.setTimerType(Qt::VeryCoarseTimer);
    m_tokenResetTimer.setInterval(TOKEN_RESET_TIMEOUT_SECONDS * 1000);
    connect(&m_tokenResetTimer, &QTimer::timeout, this, [this]() {
        if (!m_eToken.isEmpty()) {
            qCDebug(generic) << "[HotMoviesApi] Cleared cookies";
            m_eToken.clear();
        }
    });
    m_tokenResetTimer.start();
}

void HotMoviesApi::sendGetRequest(const QUrl& url, HotMoviesApi::ApiCallback callback)
{
    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);

    if (m_network.cache().hasValidElement(request)) {
        // Do not immediately run the callback because classes higher up may
        // set up a Qt connection while the network request is running.
        QTimer::singleShot(0, this, [cb = std::move(callback), element = m_network.cache().getElement(request)]() { //
            cb(element, {});
        });
        return;
    }

    confirmAge([url = url, callback = callback, this]() {
        QNetworkRequest request = mediaelch::network::requestWithDefaults(url);

        // Starting around 2024-11, HotMovies has a "confirm age" popup.
        // This only sets a simple cookie, but for some reason, we need to confirm it in MediaElch
        // nonetheless and get the cookie "etoken".
        QList<QNetworkCookie> cookies{{
            QNetworkCookie("ageConfirmed", "true"), //
            QNetworkCookie("etoken", m_eToken),
        }};
        request.setHeader(QNetworkRequest::CookieHeader, QVariant::fromValue(cookies));

        QNetworkReply* reply = m_network.getWithWatcher(request);
        connect(reply, &QNetworkReply::finished, this, [reply, cb = std::move(callback), request, this]() {
            auto dls = makeDeleteLaterScope(reply);

            QString data;
            if (reply->error() == QNetworkReply::NoError) {
                data = QString::fromUtf8(reply->readAll());

            } else {
                qCWarning(generic) << "[HotMoviesApi] Network Error:" << reply->errorString() << "for URL"
                                   << reply->url();
            }

            if (!data.isEmpty()) {
                m_network.cache().addElement(request, data);
            }

            ScraperError error = makeScraperError(data, *reply, {});
            cb(data, error);
        });
    });
}

void HotMoviesApi::confirmAge(std::function<void()> callback)
{
    if (!m_eToken.isEmpty()) {
        callback();
        return;
    }

    QUrl url = QUrl("https://www.hotmovies.com/Account/AgeConfirmation?ageConfirmationClicked=true");
    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);

    QNetworkReply* reply = m_network.getWithWatcher(request);

    connect(reply, &QNetworkReply::finished, this, [reply, cb = std::move(callback), request, this]() {
        auto dls = makeDeleteLaterScope(reply);

        if (reply->error() == QNetworkReply::NoError) {
            QVariant cookieVar = reply->header(QNetworkRequest::SetCookieHeader);
            if (cookieVar.isValid()) {
                const QList<QNetworkCookie> cookies = cookieVar.value<QList<QNetworkCookie>>();
                for (const QNetworkCookie& cookie : cookies) {
                    if (cookie.name() == "etoken") {
                        m_eToken = cookie.value();
                        break;
                    }
                }
            }

        } else {
            qCWarning(generic) << "[HotMoviesApi] Couldn't confirm age; Network Error:" << reply->errorString()
                               << "for URL" << reply->url();
        }

        cb();
    });
}

void HotMoviesApi::searchForMovie(const QString& query, HotMoviesApi::ApiCallback callback)
{
    sendGetRequest(makeMovieSearchUrl(query), std::move(callback));
}

void HotMoviesApi::loadMovie(const QString& id, HotMoviesApi::ApiCallback callback)
{
    sendGetRequest(makeMovieUrl(id), std::move(callback));
}

QUrl HotMoviesApi::makeApiUrl(const QString& suffix, QUrlQuery query) const
{
    return QStringLiteral("https://www.hotmovies.com%1?%2").arg(suffix, query.toString());
}

QUrl HotMoviesApi::makeMovieSearchUrl(const QString& searchStr) const
{
    QString encodedSearch = QUrl::toPercentEncoding(searchStr);
    QUrlQuery queries;
    queries.addQueryItem("q", encodedSearch);
    QUrl url = makeApiUrl("/adult-movies/search", queries);
    return url;
}

QUrl HotMoviesApi::makeMovieUrl(const QString& id) const
{
    return id; // the id is actually an URL
}

} // namespace scraper
} // namespace mediaelch
