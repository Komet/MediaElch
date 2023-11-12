#pragma once

#include <QString>
#include <QUrl>

namespace test {

// TODO: Refactor: referrer and other options should be passed differently. Or just return a request.
QString musicDownloadSyncOrFail(const QUrl& url, QUrl referrer = {});

} // namespace test
