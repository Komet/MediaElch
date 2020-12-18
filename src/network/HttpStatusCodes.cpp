#include "network/HttpStatusCodes.h"

namespace mediaelch {

QString translateNetworkError(QNetworkReply::NetworkError error)
{
    switch (error) {
    case QNetworkReply::OperationCanceledError: return QObject::tr("Timeout by MediaElch");
    case QNetworkReply::ContentNotFoundError: return QObject::tr("Content not found");
    case QNetworkReply::TimeoutError: return QObject::tr("Remote connection timed out");
    case QNetworkReply::SslHandshakeFailedError: return QObject::tr("SSL handshake failed");
    case QNetworkReply::TooManyRedirectsError: return QObject::tr("Too many redirects");
    case QNetworkReply::ConnectionRefusedError: return QObject::tr("Connection refused by host");
    case QNetworkReply::HostNotFoundError: return QObject::tr("Host not found");
    case QNetworkReply::UnknownNetworkError: return QObject::tr("Unknown network error");
    default: return QObject::tr("Could not load the requested resource");
    }
}

} // namespace mediaelch
