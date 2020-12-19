#include "cli/common.h"

#include <QTextStream>

// levels:
//   0  only errors
//   1  also critical messages
//   2  also warnings
//   3  also info messages
//   4  also debug messages
static int s_verbosityLevel = 0;

namespace mediaelch {
namespace cli {

MediaType mediaTypeFromString(QString str)
{
    if ("all" == str) {
        return MediaType::All;
    }
    if ("movie" == str) {
        return MediaType::Movie;
    }
    if ("tvshow" == str) {
        return MediaType::TvShow;
    }
    if ("concert" == str) {
        return MediaType::Concert;
    }
    if ("music" == str) {
        return MediaType::Music;
    }
    return MediaType::Unknown;
}

void setVerbosity(int level)
{
    if (level <= 0) {
        s_verbosityLevel = 0;
    } else if (level >= 3) {
        s_verbosityLevel = 3;
    } else {
        s_verbosityLevel = level;
    }
}

static bool shouldPrintMessage(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg: return s_verbosityLevel >= 4;
    case QtInfoMsg: return s_verbosityLevel >= 3;
    case QtWarningMsg: return s_verbosityLevel >= 2;
    case QtCriticalMsg: return s_verbosityLevel >= 1;
    case QtFatalMsg: return true;
    }
    return false;
}

void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
#ifndef QT_DEBUG
    // no debug output on the command line
    if (type == QtInfoMsg || type == QtDebugMsg) {
        return;
    }
#endif

#ifdef Q_OS_WIN32
    const QString newLine = "\r\n";
#else
    const QString newLine = "\n";
#endif

    QTextStream out(stderr);

    const auto typeStr = [type]() -> QString {
        switch (type) {
        case QtInfoMsg: return QStringLiteral("INFO:  ");
        case QtDebugMsg: return QStringLiteral("DEBUG:  ");
        case QtWarningMsg: return QStringLiteral("WARNING:  ");
        case QtCriticalMsg: return QStringLiteral("CRITICAL: ");
        case QtFatalMsg: return QStringLiteral("FATAL:    ");
        }
        return QStringLiteral("UNKNOWN:  ");
    };

    if (shouldPrintMessage(type)) {
#ifdef QT_DEBUG
        auto srcFile = QString("%1").arg(context.function, -70, QChar(' '));
        srcFile.truncate(70);
        out << "[" << srcFile << "] " << typeStr() << msg.toLocal8Bit() << newLine;
#else
        Q_UNUSED(context)
        out << "[MediaElch] " << typeStr() << msg.toLocal8Bit() << newLine;
#endif
    }

    if (type == QtFatalMsg) {
        abort();
    }
}

} // namespace cli
} // namespace mediaelch
