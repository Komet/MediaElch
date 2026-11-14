#include "test/test_helpers.h"

#include "network/NetworkRequest.h"

#include "Version.h"

using namespace mediaelch::network;

TEST_CASE("network requests", "[network]")
{
    const QUrl url("https://example.invalid/path");
    SECTION("Default requests")
    {
        const QNetworkRequest request = requestWithDefaults(url);
        CHECK(request.url() == url);
        CHECK(request.header(QNetworkRequest::UserAgentHeader).toString() == mediaelch::currentVersionIdentifier());
        CHECK(request.rawHeader("Accept").isEmpty());
        CHECK(request.header(QNetworkRequest::ContentTypeHeader).toString().isEmpty());
        CHECK(request.maximumRedirectsAllowed() == 5);
    }

    SECTION("JSON requests")
    {
        const QNetworkRequest json = jsonRequestWithDefaults(url);
        CHECK(json.url() == url);
        CHECK(json.rawHeader("Accept") == "application/json");
        CHECK(json.header(QNetworkRequest::ContentTypeHeader).toString() == "application/json");
        CHECK(json.maximumRedirectsAllowed() == 5);
    }

    SECTION("Firefox user agent")
    {
        QNetworkRequest request = requestWithDefaults(url);
        useFirefoxUserAgent(request);
        CHECK(request.header(QNetworkRequest::UserAgentHeader).toString().contains("Firefox/"));
    }
}
