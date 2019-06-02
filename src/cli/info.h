#pragma once

#include "cli/common.h"

#include <QApplication>
#include <QCommandLineParser>

namespace mediaelch {
namespace cli {

int info(QApplication& app, QCommandLineParser& parser);

} // namespace cli
} // namespace mediaelch
