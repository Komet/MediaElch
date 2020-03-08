#pragma once

#include <QNetworkRequest>

namespace mediaelch {
namespace network {

QNetworkRequest requestWithDefaults(const QUrl& url);
QNetworkRequest jsonRequestWithDefaults(const QUrl& url);

} // namespace network
} // namespace mediaelch
