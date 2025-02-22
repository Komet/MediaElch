#include "AdultDvdEmpireApi.h"

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

AdultDvdEmpireApi::AdultDvdEmpireApi(QObject* parent) : QObject(parent)
{
    m_tokenResetTimer.setTimerType(Qt::VeryCoarseTimer);
    m_tokenResetTimer.setInterval(TOKEN_RESET_TIMEOUT_SECONDS * 1000);
    connect(&m_tokenResetTimer, &QTimer::timeout, this, [this]() {
        if (!m_eToken.isEmpty()) {
            qCDebug(generic) << "[AdultDvdEmpireApi] Cleared cookies";
            m_eToken.clear();
        }
    });
    m_tokenResetTimer.start();
}

void AdultDvdEmpireApi::sendGetRequest(const QUrl& url, AdultDvdEmpireApi::ApiCallback callback)
{
    QNetworkRequest getRequest = mediaelch::network::requestWithDefaults(url);
    if (m_network.cache().hasValidElement(getRequest)) {
        // Do not immediately run the callback because classes higher up may
        // set up a Qt connection while the network request is running.
        QTimer::singleShot(
            0, this, [cb = std::move(callback), element = m_network.cache().getElement(getRequest)]() { //
                cb(element, {});
            });
        return;
    }

    confirmAge([url = url, callback = callback, this]() {
        QNetworkRequest request = mediaelch::network::requestWithDefaults(url);
        // Some server-side detection if the browser supports hovering or certain JavaScript features.
        // If we use the MediaElch user agent, then no actor images are sent in the response (i.e. HTML).
        // See GitHub issue #1164
        mediaelch::network::useFirefoxUserAgent(request);

        // Starting around 2024-11, AdultDvdEmpire has a "confirm age" popup.
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
                qCWarning(generic) << "[AdultDvdEmpireApi] Network Error:" << reply->errorString() << "for URL"
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

void AdultDvdEmpireApi::confirmAge(std::function<void()> callback)
{
    if (!m_eToken.isEmpty()) {
        callback();
        return;
    }

    QUrl url = QUrl("https://www.adultdvdempire.com/Account/AgeConfirmation?ageConfirmationClicked=true");
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
            qCWarning(generic) << "[AdultDvdEmpireApi] Couldn't confirm age; Network Error:" << reply->errorString()
                               << "for URL" << reply->url();
        }

        cb();
    });
}

void AdultDvdEmpireApi::searchForMovie(const QString& query, AdultDvdEmpireApi::ApiCallback callback)
{
    sendGetRequest(makeMovieSearchUrl(query), std::move(callback));
}

void AdultDvdEmpireApi::loadMovie(const QString& id, AdultDvdEmpireApi::ApiCallback callback)
{
    sendGetRequest(makeMovieUrl(id), std::move(callback));
}

QUrl AdultDvdEmpireApi::makeApiUrl(const QString& suffix, QUrlQuery query) const
{
    return QStringLiteral("https://www.adultdvdempire.com%1?%2").arg(suffix, query.toString());
}

QUrl AdultDvdEmpireApi::makeMovieSearchUrl(const QString& searchStr) const
{
    QUrlQuery queries;
    queries.addQueryItem("view", "list");
    queries.addQueryItem("q", searchStr);
    return makeApiUrl("/allsearch/search", queries);
}

QUrl AdultDvdEmpireApi::makeMovieUrl(const QString& id) const
{
    return makeApiUrl(id, {});
}

} // namespace scraper
} // namespace mediaelch
