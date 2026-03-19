#pragma once

#include <QApplication>
#include <QCommandLineParser>

namespace mediaelch {
namespace cli {

int checkVersionUpdate(QApplication& app, QCommandLineParser& parser);

} // namespace cli
} // namespace mediaelch
