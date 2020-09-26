#include "log/Log.h"

#include "settings/Settings.h"

#include <QMessageBox>

static QFile data;

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
#    include <unistd.h>
#endif

// Some colors
#define MEDIAELCH_FONT_RESET "\033[00m"
#define MEDIAELCH_COLOR_RED "\x1B[31m"
#define MEDIAELCH_COLOR_GREEN "\x1B[32m"
#define MEDIAELCH_COLOR_YELLOW "\x1B[33m"
#define MEDIAELCH_COLOR_BLUE "\x1B[34m"
#define MEDIAELCH_COLOR_MAGENTA "\x1B[35m"
#define MEDIAELCH_COLOR_WHITE "\x1B[37m"

#define MEDIAELCH_FONT_ITALIC "\033[3m"
#define MEDIAELCH_FONT_BOLD "\033[1m"

static bool is_stderr_tty()
{
    if (stderr == nullptr) {
        // Can't detect whether TTY or not
        return false;
    }

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    return isatty(fileno(stderr)) != 0;
#else
    // No colors for Windows, yet
    return false;
#endif
}


namespace mediaelch {

void initLoggingPattern()
{
#ifdef QT_DEBUG
    QString pattern;
    if (is_stderr_tty()) {
        pattern = "%{time yyyy-MM-dd h:mm:ss.zzz} "
                  "%{if-debug}" MEDIAELCH_COLOR_WHITE "DEBUG" MEDIAELCH_FONT_RESET "%{endif}"
                  "%{if-info}" MEDIAELCH_COLOR_BLUE " INFO" MEDIAELCH_FONT_RESET "%{endif}"
                  "%{if-warning}" MEDIAELCH_COLOR_YELLOW " WARN" MEDIAELCH_FONT_RESET "%{endif}"
                  "%{if-critical}" MEDIAELCH_COLOR_RED " CRIT" MEDIAELCH_FONT_RESET "%{endif}"
                  "%{if-fatal}" MEDIAELCH_COLOR_MAGENTA "FATAL" MEDIAELCH_FONT_RESET "%{endif}"
                  " [%{file}:%{line}] " MEDIAELCH_COLOR_BLUE "|" MEDIAELCH_FONT_RESET " %{message}";

    } else {
        pattern = "%{time yyyy-MM-dd h:mm:ss.zzz} "
                  "%{if-debug}DEBUG%{endif}"
                  "%{if-info} INFO%{endif}"
                  "%{if-warning} WARN%{endif}"
                  "%{if-critical} CRIT%{endif}"
                  "%{if-fatal}FATAL%{endif}"
                  " [%{file}:%{line}] | %{message}";
    }
#else
    QString pattern = "MediaElch %{time yyyy-MM-dd h:mm:ss.zzz} "
                      "%{if-debug}DEBUG %{endif}"
                      "%{if-info}INFO  %{endif}"
                      "%{if-warning}WARN  %{endif}"
                      "%{if-critical}CRIT  %{endif}"
                      "%{if-fatal}FATAL  %{endif}"
                      ": %{message}";

#endif
    qSetMessagePattern(pattern);
}

void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
#ifdef Q_OS_WIN
    const QString newLine = "\r\n";
#else
    const QString newLine = "\n";
#endif

    QTextStream out(stderr);
    if (data.isOpen()) {
        out.setDevice(&data);
    }

    out << qFormatLogMessage(type, context, msg) << newLine;

    if (type == QtFatalMsg) {
        abort();
    }
}

bool openLogFile(const QString& filePath)
{
    if (filePath.isEmpty()) {
        return true;
    }

    data.setFileName(filePath);

    return data.open(QFile::WriteOnly | QFile::Truncate);
}

void closeLogFile()
{
    if (data.isOpen()) {
        data.close();
    }
}

} // namespace mediaelch
