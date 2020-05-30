#include "network/Request.h"

#include "Version.h"

namespace mediaelch {
namespace network {

QNetworkRequest requestWithDefaults(const QUrl& url)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, mediaelch::currentVersionIdentifier());
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
