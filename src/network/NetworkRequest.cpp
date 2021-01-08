#include "network/NetworkRequest.h"

#include "Version.h"

namespace mediaelch {
namespace network {

QNetworkRequest requestWithDefaults(const QUrl& url)
{
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    request.setHeader(QNetworkRequest::UserAgentHeader, mediaelch::currentVersionIdentifier());
    // Default value is 50, but we have at most 2 redirects. For example:
    //  1. http://example.com/tt1234
    //  2. https://example.com/tt1234
    //  3. http://example.com/tt1234/
    // A value of 4 should be enough and is enough for all of our scrapers.
    request.setMaximumRedirectsAllowed(4);
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
