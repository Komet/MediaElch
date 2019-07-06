#pragma once

#include "cli/common.h"

#include <QApplication>
#include <QCommandLineParser>

namespace mediaelch {
namespace cli {

struct ShowConfig
{
    QString id;
};

int show(QApplication& app, QCommandLineParser& parser);

} // namespace cli
} // namespace mediaelch
