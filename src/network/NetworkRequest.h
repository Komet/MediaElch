#pragma once

#include <QNetworkRequest>

namespace mediaelch {
namespace network {

/// \brief   Create a QNetworkRequest with MediaElch-specific default values.
/// \details Defaults like the user-agent, number of allowed redirections
///          (including auto-redirection), etc. are set in this function.  Use
///          this function everywhere where you want to create a new network
//           request.
QNetworkRequest requestWithDefaults(const QUrl& url);
/// \brief   Create a QNetworkRequest which expects a JSON response and sends a JSON body.
/// \details Same as \p requestWithDefaults() except that JSON is used.
QNetworkRequest jsonRequestWithDefaults(const QUrl& url);
/// \brief   Sets the User-Agent of the request to a Firefox one.
/// \details Some pages don't work with "MediaElch" User-Agent.
///          Use this function to circumvent the limitation.
void useFirefoxUserAgent(QNetworkRequest& request);

} // namespace network
} // namespace mediaelch
