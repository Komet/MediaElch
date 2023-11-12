#include "network/NetworkRequest.h"

#include "Version.h"

namespace mediaelch {
namespace network {

QNetworkRequest requestWithDefaults(const QUrl& url)
{
    QNetworkRequest request(url);

#if QT_VERSION < QT_VERSION_CHECK(5, 9, 0)
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Default in Qt6; in Qt5, default was ManualRedirectPolicy
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
#endif
    request.setHeader(QNetworkRequest::UserAgentHeader, mediaelch::currentVersionIdentifier());
    // Default value is 50, but we have at most 2 redirects. For example:
    //  1. http://example.com/tt1234
    //  2. https://example.com/tt1234
    //  3. https://example.com/tt1234/
    // A value of 5 should be enough and is enough for all of our scrapers.
    request.setMaximumRedirectsAllowed(5);
    return request;
}

QNetworkRequest jsonRequestWithDefaults(const QUrl& url)
{
    QNetworkRequest request = requestWithDefaults(url);
    request.setRawHeader("Accept", "application/json");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return request;
}

void useFirefoxUserAgent(QNetworkRequest& request)
{
    request.setHeader(QNetworkRequest::UserAgentHeader,
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:84.0) Gecko/20100101 Firefox/84.0");
}

} // namespace network
} // namespace mediaelch
