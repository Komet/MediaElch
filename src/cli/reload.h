#pragma once

#include "cli/common.h"

#include <QApplication>
#include <QCommandLineParser>

namespace mediaelch {
namespace cli {

struct ReloadConfig
{
    MediaType mediaType = MediaType::All;
};

void reloadMovies();
void reloadTvShows();
void reloadConcerts();
void reloadMusic();

void reloadEntries(ReloadConfig config);

int reload(QApplication& app, QCommandLineParser& parser);

} // namespace cli
} // namespace mediaelch
