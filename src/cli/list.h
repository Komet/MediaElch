#pragma once

#include "cli/common.h"

#include <QApplication>
#include <QCommandLineParser>

class Album;
class Artist;
class Concert;
class Movie;
class TableWriter;
class TvShow;

namespace mediaelch {
namespace cli {

struct ListConfig
{
    MediaType mediaType = MediaType::All;
    bool reload = false;
};

void printMovie(TableWriter& table, Movie& movie);
void printConcert(TableWriter& table, Concert& concert);
void printArtist(TableWriter& table, Artist& album);
void printAlbum(TableWriter& table, Album& album);

void listMovies();
void listConcerts();
void listMusic();
void listTvShows();

void listEntries(ListConfig config);

int list(QApplication& app, QCommandLineParser& parser);

} // namespace cli
} // namespace mediaelch
