#pragma once

#include <QDebug>

namespace mediaelch {

/// \brief Sets the default message pattern of Qt's logging framework.
///
/// As per Qt documentation, the pattern can be overwritten using the
/// environment variable "QT_MESSAGE_PATTERN".  For more information see
/// https://doc.qt.io/qt-5/qtglobal.html#qSetMessagePattern
void initLoggingPattern();

/// \brief MediaElch's default message handler with optional log file.
///
/// If a debug log file is set in MediaElch's advanced settings then all debug
/// messages are redirected to that.  Otherwise stderr is used.
/// Repects QT_MESSAGE_PATTERN.
///
/// \see initLoggingPattern()
void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

/// \brief Opens the given log file for logging.
/// \returns True if the file was opened for writing successfuly.
bool openLogFile(const QString& filePath);

/// \brief Closes the currently used log file if it is opened.
void closeLogFile();

} // namespace mediaelch
