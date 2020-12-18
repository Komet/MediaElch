#pragma once

namespace mediaelch {

/// \brief Named HTTP status codes
/// \note Only those that are used in MediaElch are added here. Add more as you need.
enum class HttpStatusCode : int
{
    // Redirection
    MovedPermanently = 301,
    Found = 302,

    TooManyRequests = 429
};

} // namespace mediaelch
