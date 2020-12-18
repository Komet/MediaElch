#pragma once

#include <QNetworkReply>
#include <QString>

namespace mediaelch {

/// \brief Named HTTP status codes
/// \note Only those that are used in MediaElch are added here. Add more as you need.
enum class HttpStatusCode : int
{
    NoCode = 0,
    // Redirection
    MovedPermanently = 301,
    Found = 302,

    TooManyRequests = 429
};

/// \brief Translates the given NetworkError to a human readable error string.
QString translateNetworkError(QNetworkReply::NetworkError error);

} // namespace mediaelch
