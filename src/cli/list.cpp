#include "cli/list.h"

#include "Version.h"
#include "cli/common.h"
#include "cli/reload.h"
#include "data/concert/Concert.h"
#include "data/movie/Movie.h"
#include "data/music/Album.h"
#include "export/TableWriter.h"
#include "file_search/movie/MovieFileSearcher.h"
#include "globals/Manager.h"
#include "settings/Settings.h"

#include <iomanip>
#include <iostream>

namespace mediaelch {
namespace cli {

void printMovie(TableWriter& table, Movie& movie)
{
    table.writeCell(movie.imdbId().isValid() ? movie.imdbId().toString() : "");
    table.writeCell(movie.title());
    table.writeCell(movie.genres().join(", "));
}

void listMovies()
{
    Manager::instance()->movieFileSearcher()->setMovieDirectories(
        Settings::instance()->directorySettings().movieDirectories());

    QEventLoop loop;
    QEventLoop::connect(
        Manager::instance()->movieFileSearcher(), &mediaelch::MovieFileSearcher::finished, &loop, &QEventLoop::quit);
    Manager::instance()->movieFileSearcher()->reload(false);
    loop.exec();

    MovieModel* movieModel = Manager::instance()->movieModel();

    TableLayout layout;
    layout.addColumn(TableColumn("IMDb ID", 10));
    layout.addColumn(TableColumn("Title", 40));
    layout.addColumn(TableColumn("Genres", 30));

    std::cout << "List of all movies: \n\n";

    TableWriter table(std::cout, layout);
    table.writeHeading();

    for (int i = 0; i < movieModel->rowCount(); ++i) {
        Movie* movie = movieModel->movie(i);
        if (movie != nullptr) {
            printMovie(table, *movie);
        }
    }
}

void printConcert(TableWriter& table, Concert& concert)
{
    table.writeCell(concert.imdbId().isValid() ? concert.imdbId().toString() : "");
    table.writeCell(concert.title());
    table.writeCell(concert.genres().join(", "));
}

void listConcerts()
{
    Manager::instance()->concertFileSearcher()->setConcertDirectories(
        Settings::instance()->directorySettings().concertDirectories());
    Manager::instance()->concertFileSearcher()->reload(false);
    ConcertModel* concertModel = Manager::instance()->concertModel();

    TableLayout layout;
    layout.addColumn(TableColumn("IMDb ID", 10));
    layout.addColumn(TableColumn("Title", 30));
    layout.addColumn(TableColumn("Genres", 30));

    std::cout << "List of all concerts: \n\n";

    TableWriter table(std::cout, layout);
    table.writeHeading();

    for (int i = 0; i < concertModel->rowCount(); ++i) {
        Concert* concert = concertModel->concert(i);
        if (concert != nullptr) {
            printConcert(table, *concert);
        }
    }
}

void printAlbum(TableWriter& table, Album& album)
{
    table.writeCell(album.title());
    table.writeCell(album.genres().join(", "));
}

void printArtist(TableWriter& table, Artist& artist)
{
    for (Album* album : artist.albums()) {
        if (album != nullptr) {
            printAlbum(table, *album);
        }
    }
}

void listMusic()
{
    Manager::instance()->musicFileSearcher()->setMusicDirectories(
        Settings::instance()->directorySettings().musicDirectories());
    Manager::instance()->musicFileSearcher()->reload(false);
    MusicModel* musicModel = Manager::instance()->musicModel();

    TableLayout layout;
    layout.addColumn(TableColumn("Title", 30));
    layout.addColumn(TableColumn("Genres", 30));

    std::cout << "List of all albums: \n\n";

    TableWriter table(std::cout, layout);
    table.writeHeading();

    for (Artist* artist : musicModel->artists()) {
        if (artist != nullptr) {
            printArtist(table, *artist);
        }
    }
}

void listTvShows()
{
    Manager::instance()->tvShowFileSearcher()->setTvShowDirectories(
        Settings::instance()->directorySettings().tvShowDirectories());
    // The global TvShowFilesWidget instance is set in its constructor...
    // TODO: Don't implicitly expect that it is instantiated somewhere.
    TvShowFilesWidget filesWidget;
    Manager::instance()->tvShowFileSearcher()->reload(false);
    TvShowModel* tvShowModel = Manager::instance()->tvShowModel();

    TableLayout layout;
    layout.addColumn(TableColumn("IMDb ID", 10));
    layout.addColumn(TableColumn("TMDB ID", 7));
    layout.addColumn(TableColumn("TVmaze ID", 9));
    layout.addColumn(TableColumn("TVDb ID", 8));
    layout.addColumn(TableColumn("Title", 30));
    layout.addColumn(TableColumn("Network", 15));

    std::cout << "List of all TV shows: \n\n";

    TableWriter table(std::cout, layout);
    table.writeHeading();

    for (TvShow* show : tvShowModel->tvShows()) {
        if (show != nullptr) {
            table.writeCell(show->imdbId().toString());
            table.writeCell(show->tmdbId().toString());
            table.writeCell(show->tvmazeId().toString());
            table.writeCell(show->tvdbId().isValid() ? show->tvdbId().withPrefix() : "");
            table.writeCell(show->title());
            table.writeCell(show->networks().join(", "));
        }
    }
}

void listEntries(ListConfig config)
{
    switch (config.mediaType) {
    case MediaType::Movie: listMovies(); break;
    case MediaType::TvShow: listTvShows(); break;
    case MediaType::Concert: listConcerts(); break;
    case MediaType::Music: listMusic(); break;
    case MediaType::All:
        listMovies();
        std::cout << std::endl;
        listTvShows();
        std::cout << std::endl;
        listConcerts();
        std::cout << std::endl;
        listMusic();
        break;
    case MediaType::Unknown: break;
    }

    std::cout << std::endl;
}

int list(QApplication& app, QCommandLineParser& parser)
{
    parser.clearPositionalArguments();
    // re-add this command so that it appears when help is printed
    parser.addPositionalArgument("list", "List all media entries", "list [list_options]");

    QCommandLineOption typeOption(
        "type", R"(Media type. Either "all", "movie", "concert", "music" or "tvshow")", "mediatype", "all");

    parser.addOption(typeOption);
    parser.process(app);

    ListConfig config;
    config.mediaType = mediaTypeFromString(parser.value(typeOption));

    if (config.mediaType == MediaType::Unknown) {
        std::cerr << "Unknown media type: " << parser.value(typeOption).toStdString() << std::endl;
        return 1;
    }

    listEntries(config);

    return 1;
}

} // namespace cli
} // namespace mediaelch
