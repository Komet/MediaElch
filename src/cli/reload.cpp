#include "cli/reload.h"

#include "globals/Manager.h"
#include "movies/MovieFileSearcher.h"

#include <iostream>

namespace mediaelch {
namespace cli {

void reloadMovies()
{
    Manager::instance()->movieFileSearcher()->setMovieDirectories(
        Settings::instance()->directorySettings().movieDirectories());
    Manager::instance()->movieFileSearcher()->reload(true);
    std::cout << "Movies reloaded." << std::endl;
}

void reloadTvShows()
{
    Manager::instance()->tvShowFileSearcher()->setTvShowDirectories(
        Settings::instance()->directorySettings().tvShowDirectories());
    // The global TvShowFilesWidget instance is set in its constructor...
    // TODO: Don't implicitly expect that it is instantiated somewhere.
    TvShowFilesWidget filesWidget;
    Manager::instance()->tvShowFileSearcher()->reload(true);
    std::cout << "Concerts reloaded." << std::endl;
}

void reloadConcerts()
{
    Manager::instance()->concertFileSearcher()->setConcertDirectories(
        Settings::instance()->directorySettings().concertDirectories());
    Manager::instance()->concertFileSearcher()->reload(true);
    std::cout << "Concerts reloaded." << std::endl;
}

void reloadMusic()
{
    Manager::instance()->musicFileSearcher()->setMusicDirectories(
        Settings::instance()->directorySettings().musicDirectories());
    Manager::instance()->musicFileSearcher()->reload(true);
    std::cout << "Music reloaded." << std::endl;
}

void reloadEntries(ReloadConfig config)
{
    switch (config.mediaType) {
    case MediaType::Movie: reloadMovies(); break;
    case MediaType::TvShow: reloadTvShows(); break;
    case MediaType::Concert: reloadConcerts(); break;
    case MediaType::Music: reloadMusic(); break;
    case MediaType::All:
        reloadMovies();
        reloadTvShows();
        reloadConcerts();
        reloadMusic();
        break;
    case MediaType::Unknown: break;
    }
}

int reload(QApplication& app, QCommandLineParser& parser)
{
    parser.clearPositionalArguments();
    // re-add this command so that it appears when help is printed
    parser.addPositionalArgument("reload", "Reload all media entries from disc", "reload [reload_options]");

    QCommandLineOption typeOption(
        "type", R"(Media type. Either "all", "movie", "concert", "music" or "tvshow")", "mediatype", "all");

    parser.addOption(typeOption);
    parser.process(app);

    ReloadConfig config;
    config.mediaType = mediaTypeFromString(parser.value(typeOption));

    if (config.mediaType == MediaType::Unknown) {
        std::cerr << "Unknown media type: " << parser.value(typeOption).toStdString() << std::endl;
        return 1;
    }

    reloadEntries(config);

    return 1;
}


} // namespace cli

} // namespace mediaelch
