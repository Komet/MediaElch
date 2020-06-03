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
    request.setMaximumRedirectsAllowed(3);
    return request;
}

QNetworkRequest jsonRequestWithDefaults(const QUrl& url)
{
    QNetworkRequest request = requestWithDefaults(url);
    request.setRawHeader("Accept", "application/json");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return request;
}

} // namespace network
} // namespace mediaelch
