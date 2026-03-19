#pragma once

#include <QMessageLogContext>
#include <QString>

std::ostream& operator<<(std::ostream& os, const QString& id);

namespace mediaelch {
namespace cli {

enum class MediaType
{
    Unknown,
    All,
    Movie,
    TvShow,
    Concert,
    Music
};

MediaType mediaTypeFromString(QString str);

void setVerbosity(int level);
void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

} // namespace cli
} // namespace mediaelch
