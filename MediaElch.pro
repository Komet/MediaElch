# Check Qt versions
lessThan(QT_MAJOR_VERSION, 5): error(Qt 4 is not supported!)
equals(QT_MAJOR_VERSION, 5) {
    lessThan(QT_MINOR_VERSION, 6): error(Qt 5.6 or higher is required!)
}
equals(QT_MAJOR_VERSION, 6) {
    warning("Qt 6 has not been tested with MediaElch, yet!")
}
contains(CONFIG, USE_EXTERN_QUAZIP) {
    DEFINES += EXTERN_QUAZIP
}

!contains(DEFINES, EXTERN_QUAZIP) {
    # using internal 3rd party QUAZIP
    DEFINES += QUAZIP_BUILD
    DEFINES += QUAZIP_STATIC # Required by Quazip to export symbols
    include(third_party/quazip/quazip/quazip.pri)
}

TEMPLATE = app
TARGET = MediaElch
INCLUDEPATH += $$PWD/src
!contains(DEFINES, EXTERN_QUAZIP) {
    # using internal 3rd party QUAZIP
    INCLUDEPATH += $$PWD/third_party
}

QT += core gui network xml sql widgets multimedia multimediawidgets \
      concurrent qml quick quickwidgets opengl

CONFIG += warn_on c++14
CONFIG += lrelease embed_translations

!contains(DEFINES, EXTERN_QUAZIP) {
    # using internal 3rd party QUAZIP
    LIBS += -lz
} else {
    #using external quazip
    LIBS += -lz -lquazip5
}

!contains(CONFIG, DISABLE_UPDATER) {
    DEFINES += MEDIAELCH_UPDATER
} else {
    message("Updater disabled")
}

unix:LIBS += -lcurl
macx:LIBS += -framework Foundation
unix:!macx {
    LIBS += -ldl
}
win32 {
    DEFINES+=_UNICODE
    DEFINES+= ZLIB_WINAPI
}

*-g++*|*-clang* {
    # Include all Qt modules using isystem so that warnings reagarding Qt files are ignored
    QMAKE_CXXFLAGS += -isystem "$$[QT_INSTALL_HEADERS]"
    for (inc, QT) {
        QMAKE_CXXFLAGS += -isystem \"$$[QT_INSTALL_HEADERS]/Qt$$system("echo $$inc | sed 's/.*/\u&/'")\"
    }
}

# Usage as: CONFIG+=ubsan
ubsan {
    message("using ubsan")
    QMAKE_CXXFLAGS += -fsanitize=undefined
    QMAKE_LFLAGS += -fsanitize=undefined
    LIBS += -lubsan
}

# Enable (all/most) warnings but ignore them for quazip files.
*-g++* {
    WARNINGS += -Wall -Wextra -pedantic
    WARNINGS += -Wunknown-pragmas -Wundef -Wold-style-cast -Wuseless-cast
    WARNINGS += -Wdisabled-optimization -Wstrict-overflow=4
    WARNINGS += -Winit-self -Wpointer-arith
    WARNINGS += -Wlogical-op -Wunsafe-loop-optimizations
    WARNINGS += -Wshadow
    WARNINGS += -Wno-error=unsafe-loop-optimizations
    WARNINGS += -Wno-unsafe-loop-optimizations
    WARNINGS += -Wno-strict-overflow
}
*-clang* {
    WARNINGS += -Wextra
}
QUAZIP_FILES = qua% qioapi% zip% unzip%
QMAKE_CXXFLAGS_WARN_ON += $(and $(filter-out moc_% qrc_% $$QUAZIP_FILES, $@),$${WARNINGS})

target.path = /usr/bin
INSTALLS += target

dotDesktop.path = /usr/share/applications
dotDesktop.files = data/desktop/MediaElch.desktop
INSTALLS += dotDesktop

icon.path = /usr/share/pixmaps
icon.files = data/desktop/MediaElch.png
INSTALLS += icon

ICON = MediaElch.icns
RC_FILE = MediaElch.rc
QMAKE_INFO_PLIST = MediaElch.plist

SOURCES += src/main.cpp \
    src/concerts/ConcertController.cpp \
    src/data/MediaInfoFile.cpp \
    src/export/CsvExport.cpp \
    src/globals/Containers.cpp \
    src/globals/Random.cpp \
    src/music/AllMusicId.cpp \
    src/music/MusicBrainzId.cpp \
    src/music/TheAudioDbId.cpp \
    src/network/HttpStatusCodes.cpp \
    src/network/NetworkRequest.cpp \
    src/network/NetworkManager.cpp \
    src/scrapers/ScraperError.cpp \
    src/scrapers/music/AllMusic.cpp \
    src/scrapers/music/Discogs.cpp \
    src/scrapers/music/MusicBrainz.cpp \
    src/scrapers/music/TheAudioDb.cpp \
    src/ui/concerts/ConcertFilesWidget.cpp \
    src/ui/concerts/ConcertSearch.cpp \
    src/ui/concerts/ConcertSearchWidget.cpp \
    src/ui/concerts/ConcertStreamDetailsWidget.cpp \
    src/ui/concerts/ConcertWidget.cpp \
    src/ui/concerts/ConcertInfoWidget.cpp \
    src/concerts/Concert.cpp \
    src/concerts/ConcertFileSearcher.cpp \
    src/concerts/ConcertModel.cpp \
    src/concerts/ConcertProxyModel.cpp \
    src/data/Database.cpp \
    src/data/ImageCache.cpp \
    src/data/ResumeTime.cpp \
    src/movies/Movie.cpp \
    src/movies/file_searcher/MovieFileSearcher.cpp \
    src/movies/file_searcher/MovieDirectorySearcher.cpp \
    src/movies/MovieFilesOrganizer.cpp \
    src/movies/MovieImages.cpp \
    src/movies/MovieModel.cpp \
    src/movies/MovieProxyModel.cpp \
    src/data/Locale.cpp \
    src/data/Rating.cpp \
    src/data/Storage.cpp \
    src/data/StreamDetails.cpp \
    src/data/Subtitle.cpp \
    src/tv_shows/TvShow.cpp \
    src/tv_shows/TvShowEpisode.cpp \
    src/tv_shows/TvShowFileSearcher.cpp \
    src/ui/export/CsvExportDialog.cpp \
    src/ui/imports/ImportActions.cpp \
    src/ui/imports/ImportDialog.cpp \
    src/ui/imports/DownloadsWidget.cpp \
    src/ui/imports/MakeMkvDialog.cpp \
    src/ui/export/ExportDialog.cpp \
    src/ui/imports/UnpackButtons.cpp \
    src/imports/MakeMkvCon.cpp \
    src/imports/MyFile.cpp \
    src/imports/Extractor.cpp \
    src/imports/FileWorker.cpp \
    src/imports/DownloadFileSearcher.cpp \
    src/log/Log.cpp \
    src/export/ExportTemplate.cpp \
    src/export/ExportTemplateLoader.cpp \
    src/export/MediaExport.cpp \
    src/export/SimpleEngine.cpp \
    src/file/FileFilter.cpp \
    src/file/FilenameUtils.cpp \
    src/file/Path.cpp \
    src/globals/Actor.cpp \
    src/globals/ComboDelegate.cpp \
    src/globals/DownloadManager.cpp \
    src/globals/DownloadManagerElement.cpp \
    src/globals/Filter.cpp \
    src/globals/Globals.cpp \
    src/globals/Helper.cpp \
    src/globals/ImageDialog.cpp \
    src/globals/ImagePreviewDialog.cpp \
    src/globals/JsonRequest.cpp \
    src/globals/Manager.cpp \
    src/globals/MessageIds.cpp \
    src/globals/Math.cpp \
    src/globals/Meta.cpp \
    src/file/NameFormatter.cpp \
    src/network/NetworkReplyWatcher.cpp \
    src/network/WebsiteCache.cpp \
    src/globals/Poster.cpp \
    src/globals/ScraperInfos.cpp \
    src/globals/ScraperManager.cpp \
    src/globals/ScraperResult.cpp \
    src/globals/Time.cpp \
    src/globals/TrailerDialog.cpp \
    src/globals/VersionInfo.cpp \
    src/image/Image.cpp \
    src/image/ImageCapture.cpp \
    src/image/ImageModel.cpp \
    src/image/ImageProxyModel.cpp \
    src/image/ThumbnailDimensions.cpp \
    src/ui/image/ImageWidget.cpp \
    src/ui/main/AboutDialog.cpp \
    src/ui/main/FileScannerDialog.cpp \
    src/ui/main/MainWindow.cpp \
    src/ui/main/Message.cpp \
    src/ui/main/MyIconFont.cpp \
    src/ui/main/Navbar.cpp \
    src/ui/main/QuickOpen.cpp \
    src/ui/main/Update.cpp \
    src/media_centers/kodi/KodiXmlWriter.cpp \
    src/media_centers/kodi/AlbumXmlReader.cpp \
    src/media_centers/kodi/AlbumXmlWriter.cpp \
    src/media_centers/kodi/ArtistXmlReader.cpp \
    src/media_centers/kodi/ArtistXmlWriter.cpp \
    src/media_centers/kodi/ConcertXmlReader.cpp \
    src/media_centers/kodi/ConcertXmlWriter.cpp \
    src/media_centers/kodi/EpisodeXmlWriter.cpp \
    src/media_centers/kodi/EpisodeXmlReader.cpp \
    src/media_centers/kodi/MovieXmlReader.cpp \
    src/media_centers/kodi/MovieXmlWriter.cpp \
    src/media_centers/kodi/TvShowXmlReader.cpp \
    src/media_centers/kodi/TvShowXmlWriter.cpp \
    src/media_centers/KodiVersion.cpp \
    src/media_centers/KodiXml.cpp \
    src/ui/movies/CertificationWidget.cpp \
    src/ui/movies/GenreWidget.cpp \
    src/movies/MovieController.cpp \
    src/ui/movies/MovieDuplicateItem.cpp \
    src/ui/movies/MovieDuplicates.cpp \
    src/ui/movies/MovieFilesWidget.cpp \
    src/ui/movies/MovieMultiScrapeDialog.cpp \
    src/ui/movies/MovieSearch.cpp \
    src/ui/movies/MovieSearchWidget.cpp \
    src/ui/movies/MovieWidget.cpp \
    src/music/Album.cpp \
    src/music/AlbumController.cpp \
    src/music/Artist.cpp \
    src/music/ArtistController.cpp \
    src/music/MusicFileSearcher.cpp \
    src/ui/music/MusicFilesWidget.cpp \
    src/music/MusicModel.cpp \
    src/music/MusicModelItem.cpp \
    src/ui/music/MusicMultiScrapeDialog.cpp \
    src/music/MusicProxyModel.cpp \
    src/ui/music/MusicSearch.cpp \
    src/ui/music/MusicSearchWidget.cpp \
    src/ui/music/MusicWidget.cpp \
    src/ui/music/MusicWidgetAlbum.cpp \
    src/ui/music/MusicWidgetArtist.cpp \
    src/ui/notifications/NotificationBox.cpp \
    src/ui/notifications/Notificator.cpp \
    src/qml/AlbumImageProvider.cpp \
    src/renamer/ConcertRenamer.cpp \
    src/renamer/EpisodeRenamer.cpp \
    src/renamer/MovieRenamer.cpp \
    src/renamer/Renamer.cpp \
    src/renamer/RenamerDialog.cpp \
    src/renamer/RenamerPlaceholders.cpp \
    src/scrapers/tmdb/TmdbApi.cpp \
    src/scrapers/ScraperInterface.cpp \
    src/scrapers/image/FanartTv.cpp \
    src/scrapers/image/FanartTvMusic.cpp \
    src/scrapers/image/FanartTvMusicArtists.cpp \
    src/scrapers/image/TheTvDbImages.cpp \
    src/scrapers/image/TMDbImages.cpp \
    src/scrapers/concert/ConcertIdentifier.cpp \
    src/scrapers/concert/ConcertScraper.cpp \
    src/scrapers/concert/ConcertSearchJob.cpp \
    src/scrapers/concert/tmdb/TmdbConcert.cpp \
    src/scrapers/concert/tmdb/TmdbConcertSearchJob.cpp \
    src/scrapers/movie/adultdvdempire/AdultDvdEmpire.cpp \
    src/scrapers/movie/adultdvdempire/AdultDvdEmpireApi.cpp \
    src/scrapers/movie/aebn/AEBN.cpp \
    src/scrapers/movie/aebn/AebnApi.cpp \
    src/scrapers/movie/custom/CustomMovieScraper.cpp \
    src/scrapers/movie/hotmovies/HotMovies.cpp \
    src/scrapers/movie/hotmovies/HotMoviesApi.cpp \
    src/scrapers/movie/imdb/ImdbMovie.cpp \
    src/scrapers/movie/imdb/ImdbMovieScraper.cpp \
    src/scrapers/movie/MovieScraper.cpp \
    src/scrapers/music/MusicScraper.cpp \
    src/scrapers/movie/ofdb/OFDb.cpp \
    src/scrapers/movie/ofdb/OfdbApi.cpp \
    src/scrapers/movie/tmdb/TmdbMovie.cpp \
    src/scrapers/movie/videobuster/VideoBuster.cpp \
    src/scrapers/movie/videobuster/VideoBusterApi.cpp \
    src/scrapers/music/TvTunes.cpp \
    src/scrapers/music/UniversalMusicScraper.cpp \
    src/scrapers/tv_show/ShowIdentifier.cpp \
    src/scrapers/tv_show/EpisodeIdentifier.cpp \
    src/scrapers/tv_show/TvScraper.cpp \
    src/scrapers/tv_show/EpisodeScrapeJob.cpp \
    src/scrapers/tv_show/ShowScrapeJob.cpp \
    src/scrapers/tv_show/ShowSearchJob.cpp \
    src/scrapers/tv_show/SeasonScrapeJob.cpp \
    src/scrapers/tv_show/ShowMerger.cpp \
    src/scrapers/tv_show/empty/EmptyTvScraper.cpp \
    src/scrapers/tv_show/custom/CustomTvScraper.cpp \
    src/scrapers/tv_show/custom/CustomTvScraperConfig.cpp \
    src/scrapers/tv_show/custom/CustomEpisodeScrapeJob.cpp \
    src/scrapers/tv_show/custom/CustomSeasonScrapeJob.cpp \
    src/scrapers/tv_show/custom/CustomShowScrapeJob.cpp \
    src/scrapers/tv_show/imdb/ImdbTv.cpp \
    src/scrapers/imdb/ImdbApi.cpp \
    src/scrapers/tv_show/imdb/ImdbTvEpisodeParser.cpp \
    src/scrapers/tv_show/imdb/ImdbTvEpisodeScrapeJob.cpp \
    src/scrapers/tv_show/imdb/ImdbTvShowSearchJob.cpp \
    src/scrapers/tv_show/imdb/ImdbTvShowParser.cpp \
    src/scrapers/tv_show/imdb/ImdbTvShowScrapeJob.cpp \
    src/scrapers/tv_show/imdb/ImdbTvSeasonParser.cpp \
    src/scrapers/tv_show/imdb/ImdbTvSeasonScrapeJob.cpp \
    src/scrapers/tv_show/thetvdb/TheTvDb.cpp \
    src/scrapers/tv_show/thetvdb/TheTvDbApi.cpp \
    src/scrapers/tv_show/thetvdb/TheTvDbEpisodeScrapeJob.cpp \
    src/scrapers/tv_show/thetvdb/TheTvDbEpisodeParser.cpp \
    src/scrapers/tv_show/thetvdb/TheTvDbEpisodesParser.cpp \
    src/scrapers/tv_show/thetvdb/TheTvDbShowSearchJob.cpp \
    src/scrapers/tv_show/thetvdb/TheTvDbShowScrapeJob.cpp \
    src/scrapers/tv_show/thetvdb/TheTvDbSeasonScrapeJob.cpp \
    src/scrapers/tv_show/thetvdb/TheTvDbShowParser.cpp \
    src/scrapers/tv_show/tvmaze/TvMaze.cpp \
    src/scrapers/tv_show/tvmaze/TvMazeApi.cpp \
    src/scrapers/tv_show/tvmaze/TvMazeEpisodeScrapeJob.cpp \
    src/scrapers/tv_show/tvmaze/TvMazeEpisodeParser.cpp \
    src/scrapers/tv_show/tvmaze/TvMazeShowSearchJob.cpp \
    src/scrapers/tv_show/tvmaze/TvMazeShowScrapeJob.cpp \
    src/scrapers/tv_show/tvmaze/TvMazeSeasonScrapeJob.cpp \
    src/scrapers/tv_show/tvmaze/TvMazeShowParser.cpp \
    src/scrapers/tv_show/tmdb/TmdbTv.cpp \
    src/scrapers/tv_show/tmdb/TmdbTvEpisodeScrapeJob.cpp \
    src/scrapers/tv_show/tmdb/TmdbTvShowSearchJob.cpp \
    src/scrapers/tv_show/tmdb/TmdbTvShowParser.cpp \
    src/scrapers/tv_show/tmdb/TmdbTvEpisodeParser.cpp \
    src/scrapers/tv_show/tmdb/TmdbTvSeasonParser.cpp \
    src/scrapers/tv_show/tmdb/TmdbTvShowScrapeJob.cpp \
    src/scrapers/tv_show/tmdb/TmdbTvSeasonScrapeJob.cpp \
    src/ui/movie_sets/MovieListDialog.cpp \
    src/ui/movie_sets/SetsWidget.cpp \
    src/settings/AdvancedSettings.cpp \
    src/settings/AdvancedSettingsXmlReader.cpp \
    src/settings/DataFile.cpp \
    src/settings/DirectorySettings.cpp \
    src/ui/settings/ExportTemplateWidget.cpp \
    src/settings/ImportSettings.cpp \
    src/settings/KodiSettings.cpp \
    src/settings/NetworkSettings.cpp \
    src/settings/ScraperSettings.cpp \
    src/settings/Settings.cpp \
    src/ui/settings/SettingsWindow.cpp \
    src/ui/settings/ConcertSettingsWidget.cpp \
    src/ui/settings/GlobalSettingsWidget.cpp \
    src/ui/settings/ExportSettingsWidget.cpp \
    src/ui/settings/ImportSettingsWidget.cpp \
    src/ui/settings/KodiSettingsWidget.cpp \
    src/ui/settings/MovieSettingsWidget.cpp \
    src/ui/settings/MusicSettingsWidget.cpp \
    src/ui/settings/NetworkSettingsWidget.cpp \
    src/ui/settings/ScraperSettingsWidget.cpp \
    src/ui/settings/TvScraperSettingsWidget.cpp \
    src/ui/settings/CustomTvScraperSettingsWidget.cpp \
    src/ui/settings/TvShowSettingsWidget.cpp \
    src/ui/small_widgets/AlphabeticalList.cpp \
    src/ui/small_widgets/Badge.cpp \
    src/ui/small_widgets/ClosableImage.cpp \
    src/ui/small_widgets/FilterWidget.cpp \
    src/ui/small_widgets/ImageGallery.cpp \
    src/ui/small_widgets/ImageLabel.cpp \
    src/ui/small_widgets/LanguageCombo.cpp \
    src/ui/small_widgets/LoadingStreamDetails.cpp \
    src/ui/small_widgets/MediaFlags.cpp \
    src/ui/small_widgets/MessageLabel.cpp \
    src/ui/small_widgets/MusicTreeView.cpp \
    src/ui/small_widgets/MyCheckBox.cpp \
    src/ui/small_widgets/MyLabel.cpp \
    src/ui/small_widgets/MyLineEdit.cpp \
    src/ui/small_widgets/MySpinBox.cpp \
    src/ui/small_widgets/MySplitter.cpp \
    src/ui/small_widgets/MySplitterHandle.cpp \
    src/ui/small_widgets/MyTableView.cpp \
    src/ui/small_widgets/MyTableWidget.cpp \
    src/ui/small_widgets/MyTableWidgetItem.cpp \
    src/ui/small_widgets/MyTreeView.cpp \
    src/ui/small_widgets/MyWidget.cpp \
    src/ui/small_widgets/SearchOverlay.cpp \
    src/ui/small_widgets/SlidingStackedWidget.cpp \
    src/ui/small_widgets/TagCloud.cpp \
    src/ui/small_widgets/TvShowTreeView.cpp \
    src/ui/tv_show/TvShowFilesWidget.cpp \
    src/ui/support/SupportDialog.cpp \
    src/scrapers/trailer/HdTrailers.cpp \
    src/ui/tv_show/TvShowCommonWidgets.cpp \
    src/ui/tv_show/TvShowMultiScrapeDialog.cpp \
    src/ui/tv_show/TvShowSearch.cpp \
    src/ui/tv_show/TvShowSearchWidget.cpp \
    src/tv_shows/TvShowUpdater.cpp \
    src/tv_shows/TvShowUtils.cpp \
    src/ui/tv_show/TvShowWidget.cpp \
    src/ui/tv_show/TvShowWidgetEpisode.cpp \
    src/ui/tv_show/TvShowWidgetSeason.cpp \
    src/ui/tv_show/TvShowWidgetTvShow.cpp \
    src/ui/tv_show/TvTunesDialog.cpp \
    src/tv_shows/EpisodeMap.cpp \
    src/tv_shows/TvShowModel.cpp \
    src/tv_shows/TvShowProxyModel.cpp \
    src/tv_shows/model/TvShowModelItem.cpp \
    src/tv_shows/model/TvShowBaseModelItem.cpp \
    src/tv_shows/model/TvShowRootModelItem.cpp \
    src/tv_shows/model/EpisodeModelItem.cpp \
    src/tv_shows/model/SeasonModelItem.cpp \
    src/ui/media_centers/KodiSync.cpp \
    src/data/ImdbId.cpp \
    src/data/TmdbId.cpp \
    src/tv_shows/TvDbId.cpp \
    src/tv_shows/TvMazeId.cpp \
    src/tv_shows/EpisodeNumber.cpp \
    src/tv_shows/SeasonNumber.cpp \
    src/tv_shows/SeasonOrder.cpp \
    src/data/Certification.cpp \
    src/movies/MovieCrew.cpp \
    src/movies/MovieSet.cpp \
    src/scrapers/movie/MovieIdentifier.cpp

macx {
    OBJECTIVE_SOURCES += src/ui/notifications/MacNotificationHandler.mm
}

HEADERS  += Version.h \
    src/concerts/ConcertController.h \
    src/data/MediaInfoFile.h \
    src/export/CsvExport.h \
    src/globals/Containers.h \
    src/globals/Random.h \
    src/music/AllMusicId.h \
    src/music/MusicBrainzId.h \
    src/music/TheAudioDbId.h \
    src/network/HttpStatusCodes.h \
    src/network/NetworkRequest.h \
    src/network/NetworkManager.h \
    src/scrapers/ScraperError.h \
    src/scrapers/music/AllMusic.h \
    src/scrapers/music/Discogs.h \
    src/scrapers/music/MusicBrainz.h \
    src/scrapers/music/TheAudioDb.h \
    src/ui/concerts/ConcertFilesWidget.h \
    src/ui/concerts/ConcertSearch.h \
    src/ui/concerts/ConcertSearchWidget.h \
    src/ui/concerts/ConcertWidget.h \
    src/ui/concerts/ConcertInfoWidget.h \
    src/concerts/Concert.h \
    src/concerts/ConcertFileSearcher.h \
    src/concerts/ConcertModel.h \
    src/concerts/ConcertProxyModel.h \
    src/ui/concerts/ConcertStreamDetailsWidget.h \
    src/data/Database.h \
    src/data/ImageCache.h \
    src/data/ResumeTime.h \
    src/media_centers/MediaCenterInterface.h \
    src/movies/Movie.h \
    src/movies/file_searcher/MovieFileSearcher.h \
    src/movies/file_searcher/MovieDirectorySearcher.h \
    src/movies/MovieFilesOrganizer.h \
    src/movies/MovieImages.h \
    src/movies/MovieModel.h \
    src/movies/MovieProxyModel.h \
    src/scrapers/image/ImageProvider.h \
    src/scrapers/concert/ConcertIdentifier.h \
    src/scrapers/concert/ConcertScraper.h \
    src/scrapers/concert/ConcertSearchJob.h \
    src/scrapers/music/MusicScraper.h \
    src/scrapers/movie/MovieScraper.h \
    src/scrapers/ScraperInterface.h \
    src/data/Locale.h \
    src/data/Rating.h \
    src/data/Storage.h \
    src/data/StreamDetails.h \
    src/data/Subtitle.h \
    src/tv_shows/TvShow.h \
    src/tv_shows/TvShowEpisode.h \
    src/tv_shows/TvShowFileSearcher.h \
    src/imports/DownloadFileSearcher.h \
    src/imports/Extractor.h \
    src/imports/FileWorker.h \
    src/imports/MakeMkvCon.h \
    src/imports/MyFile.h \
    src/log/Log.h \
    src/ui/export/CsvExportDialog.h \
    src/ui/export/ExportDialog.h \
    src/ui/imports/DownloadsWidget.h \
    src/ui/imports/ImportActions.h \
    src/ui/imports/ImportDialog.h \
    src/ui/imports/MakeMkvDialog.h \
    src/ui/imports/UnpackButtons.h \
    src/export/ExportTemplate.h \
    src/export/ExportTemplateLoader.h \
    src/export/MediaExport.h \
    src/export/SimpleEngine.h \
    src/file/FileFilter.h \
    src/file/FilenameUtils.h \
    src/file/Path.h \
    src/globals/Actor.h \
    src/globals/ComboDelegate.h \
    src/globals/DownloadManager.h \
    src/globals/DownloadManagerElement.h \
    src/globals/Filter.h \
    src/globals/Globals.h \
    src/globals/Helper.h \
    src/globals/ImageDialog.h \
    src/globals/ImagePreviewDialog.h \
    src/globals/JsonRequest.h \
    src/globals/LocaleStringCompare.h \
    src/globals/Manager.h \
    src/globals/MessageIds.h \
    src/globals/Math.h \
    src/globals/Meta.h \
    src/file/NameFormatter.h \
    src/network/NetworkReplyWatcher.h \
    src/network/WebsiteCache.h \
    src/globals/Poster.h \
    src/globals/ScraperInfos.h \
    src/globals/ScraperManager.h \
    src/globals/ScraperResult.h \
    src/globals/Time.h \
    src/globals/TrailerDialog.h \
    src/globals/VersionInfo.h \
    src/image/Image.h \
    src/image/ImageCapture.h \
    src/image/ImageModel.h \
    src/image/ImageProxyModel.h \
    src/image/ThumbnailDimensions.h \
    src/ui/image/ImageWidget.h \
    src/scrapers/image/FanartTv.h \
    src/scrapers/image/FanartTvMusic.h \
    src/scrapers/image/FanartTvMusicArtists.h \
    src/scrapers/image/TheTvDbImages.h \
    src/scrapers/image/TMDbImages.h \
    src/ui/main/AboutDialog.h \
    src/ui/main/FileScannerDialog.h \
    src/ui/main/MainWindow.h \
    src/ui/main/Message.h \
    src/ui/main/MyIconFont.h \
    src/ui/main/Navbar.h \
    src/ui/main/QuickOpen.h \
    src/ui/main/Update.h \
    src/media_centers/kodi/KodiXmlWriter.h \
    src/media_centers/kodi/AlbumXmlReader.h \
    src/media_centers/kodi/AlbumXmlWriter.h \
    src/media_centers/kodi/ArtistXmlReader.h \
    src/media_centers/kodi/ArtistXmlWriter.h \
    src/media_centers/kodi/ConcertXmlReader.h \
    src/media_centers/kodi/ConcertXmlWriter.h \
    src/media_centers/kodi/EpisodeXmlWriter.h \
    src/media_centers/kodi/EpisodeXmlReader.h \
    src/media_centers/kodi/MovieXmlReader.h \
    src/media_centers/kodi/MovieXmlWriter.h \
    src/media_centers/kodi/TvShowXmlReader.h \
    src/media_centers/kodi/TvShowXmlWriter.h \
    src/media_centers/KodiVersion.h \
    src/media_centers/KodiVersion.h \
    src/media_centers/KodiXml.h \
    src/ui/movies/CertificationWidget.h \
    src/ui/movies/GenreWidget.h \
    src/movies/MovieController.h \
    src/ui/movies/MovieDuplicateItem.h \
    src/ui/movies/MovieDuplicates.h \
    src/ui/movies/MovieFilesWidget.h \
    src/ui/movies/MovieMultiScrapeDialog.h \
    src/ui/movies/MovieSearch.h \
    src/ui/movies/MovieSearchWidget.h \
    src/ui/movies/MovieWidget.h \
    src/music/Album.h \
    src/music/AlbumController.h \
    src/music/Artist.h \
    src/music/ArtistController.h \
    src/music/MusicFileSearcher.h \
    src/ui/music/MusicFilesWidget.h \
    src/music/MusicModel.h \
    src/music/MusicModelItem.h \
    src/ui/music/MusicMultiScrapeDialog.h \
    src/music/MusicProxyModel.h \
    src/ui/music/MusicSearch.h \
    src/ui/music/MusicSearchWidget.h \
    src/ui/music/MusicWidget.h \
    src/ui/music/MusicWidgetAlbum.h \
    src/ui/music/MusicWidgetArtist.h \
    src/ui/notifications/MacNotificationHandler.h \
    src/ui/notifications/NotificationBox.h \
    src/ui/notifications/Notificator.h \
    src/qml/AlbumImageProvider.h \
    src/renamer/ConcertRenamer.h \
    src/renamer/EpisodeRenamer.h \
    src/renamer/MovieRenamer.h \
    src/renamer/Renamer.h \
    src/renamer/RenamerDialog.h \
    src/renamer/RenamerPlaceholders.h \
    src/scrapers/tmdb/TmdbApi.h \
    src/scrapers/concert/tmdb/TmdbConcert.h \
    src/scrapers/concert/tmdb/TmdbConcertSearchJob.h \
    src/scrapers/movie/adultdvdempire/AdultDvdEmpire.h \
    src/scrapers/movie/adultdvdempire/AdultDvdEmpireApi.h \
    src/scrapers/movie/aebn/AEBN.h \
    src/scrapers/movie/aebn/AebnApi.h \
    src/scrapers/movie/custom/CustomMovieScraper.h \
    src/scrapers/movie/hotmovies/HotMovies.h \
    src/scrapers/movie/hotmovies/HotMoviesApi.h \
    src/scrapers/movie/imdb/ImdbMovie.h \
    src/scrapers/movie/imdb/ImdbMovieScraper.h \
    src/scrapers/movie/ofdb/OFDb.h \
    src/scrapers/movie/ofdb/OfdbApi.h \
    src/scrapers/movie/tmdb/TmdbMovie.h \
    src/scrapers/movie/videobuster/VideoBuster.h \
    src/scrapers/movie/videobuster/VideoBusterApi.h \
    src/scrapers/music/TvTunes.h \
    src/scrapers/music/UniversalMusicScraper.h \
    src/scrapers/tv_show/ShowIdentifier.h \
    src/scrapers/tv_show/EpisodeIdentifier.h \
    src/scrapers/tv_show/TvScraper.h \
    src/scrapers/tv_show/EpisodeScrapeJob.h \
    src/scrapers/tv_show/ShowScrapeJob.h \
    src/scrapers/tv_show/ShowSearchJob.h \
    src/scrapers/tv_show/SeasonScrapeJob.h \
    src/scrapers/tv_show/ShowMerger.h \
    src/scrapers/tv_show/empty/EmptyTvScraper.h \
    src/scrapers/tv_show/custom/CustomTvScraper.h \
    src/scrapers/tv_show/custom/CustomTvScraperConfig.h \
    src/scrapers/tv_show/custom/CustomSeasonScrapeJob.h \
    src/scrapers/tv_show/custom/CustomEpisodeScrapeJob.h \
    src/scrapers/tv_show/custom/CustomShowScrapeJob.h \
    src/scrapers/tv_show/imdb/ImdbTv.h \
    src/scrapers/imdb/ImdbApi.h \
    src/scrapers/tv_show/imdb/ImdbTvEpisodeParser.h \
    src/scrapers/tv_show/imdb/ImdbTvEpisodeScrapeJob.h \
    src/scrapers/tv_show/imdb/ImdbTvShowSearchJob.h \
    src/scrapers/tv_show/imdb/ImdbTvShowParser.h \
    src/scrapers/tv_show/imdb/ImdbTvShowScrapeJob.h \
    src/scrapers/tv_show/imdb/ImdbTvSeasonScrapeJob.h \
    src/scrapers/tv_show/imdb/ImdbTvSeasonParser.h \
    src/scrapers/tv_show/thetvdb/TheTvDb.h \
    src/scrapers/tv_show/thetvdb/TheTvDbApi.h \
    src/scrapers/tv_show/thetvdb/TheTvDbEpisodeScrapeJob.h \
    src/scrapers/tv_show/thetvdb/TheTvDbEpisodeParser.h \
    src/scrapers/tv_show/thetvdb/TheTvDbEpisodesParser.h \
    src/scrapers/tv_show/thetvdb/TheTvDbShowSearchJob.h \
    src/scrapers/tv_show/thetvdb/TheTvDbShowScrapeJob.h \
    src/scrapers/tv_show/thetvdb/TheTvDbSeasonScrapeJob.h \
    src/scrapers/tv_show/thetvdb/TheTvDbShowParser.h \
    src/scrapers/tv_show/tvmaze/TvMaze.h \
    src/scrapers/tv_show/tvmaze/TvMazeApi.h \
    src/scrapers/tv_show/tvmaze/TvMazeEpisodeScrapeJob.h \
    src/scrapers/tv_show/tvmaze/TvMazeEpisodeParser.h \
    src/scrapers/tv_show/tvmaze/TvMazeShowSearchJob.h \
    src/scrapers/tv_show/tvmaze/TvMazeShowScrapeJob.h \
    src/scrapers/tv_show/tvmaze/TvMazeSeasonScrapeJob.h \
    src/scrapers/tv_show/tvmaze/TvMazeShowParser.h \
    src/scrapers/tv_show/tmdb/TmdbTv.h \
    src/scrapers/tv_show/tmdb/TmdbTvEpisodeScrapeJob.h \
    src/scrapers/tv_show/tmdb/TmdbTvShowSearchJob.h \
    src/scrapers/tv_show/tmdb/TmdbTvShowParser.h \
    src/scrapers/tv_show/tmdb/TmdbTvEpisodeParser.h \
    src/scrapers/tv_show/tmdb/TmdbTvSeasonParser.h \
    src/scrapers/tv_show/tmdb/TmdbTvShowScrapeJob.h \
    src/scrapers/tv_show/tmdb/TmdbTvSeasonScrapeJob.h \
    src/ui/movie_sets/MovieListDialog.h \
    src/ui/movie_sets/SetsWidget.h \
    src/settings/AdvancedSettings.h \
    src/settings/AdvancedSettingsXmlReader.h \
    src/settings/DataFile.h \
    src/settings/DirectorySettings.h \
    src/ui/settings/ExportTemplateWidget.h \
    src/settings/ImportSettings.h \
    src/settings/KodiSettings.h \
    src/settings/NetworkSettings.h \
    src/settings/ScraperSettings.h \
    src/settings/Settings.h \
    src/ui/settings/SettingsWindow.h \
    src/ui/settings/ConcertSettingsWidget.h \
    src/ui/settings/GlobalSettingsWidget.h \
    src/ui/settings/ExportSettingsWidget.h \
    src/ui/settings/ImportSettingsWidget.h \
    src/ui/settings/KodiSettingsWidget.h \
    src/ui/settings/MovieSettingsWidget.h \
    src/ui/settings/MusicSettingsWidget.h \
    src/ui/settings/NetworkSettingsWidget.h \
    src/ui/settings/ScraperSettingsWidget.h \
    src/ui/settings/TvScraperSettingsWidget.h \
    src/ui/settings/CustomTvScraperSettingsWidget.h \
    src/ui/settings/TvShowSettingsWidget.h \
    src/ui/small_widgets/AlphabeticalList.h \
    src/ui/small_widgets/Badge.h \
    src/ui/small_widgets/ClosableImage.h \
    src/ui/small_widgets/FilterWidget.h \
    src/ui/small_widgets/ImageGallery.h \
    src/ui/small_widgets/ImageLabel.h \
    src/ui/small_widgets/LanguageCombo.h \
    src/ui/small_widgets/LoadingStreamDetails.h \
    src/ui/small_widgets/MediaFlags.h \
    src/ui/small_widgets/MessageLabel.h \
    src/ui/small_widgets/MusicTreeView.h \
    src/ui/small_widgets/MyCheckBox.h \
    src/ui/small_widgets/MyLabel.h \
    src/ui/small_widgets/MyLineEdit.h \
    src/ui/small_widgets/MySpinBox.h \
    src/ui/small_widgets/MySplitter.h \
    src/ui/small_widgets/MySplitterHandle.h \
    src/ui/small_widgets/MyTableView.h \
    src/ui/small_widgets/MyTableWidget.h \
    src/ui/small_widgets/MyTableWidgetItem.h \
    src/ui/small_widgets/MyTreeView.h \
    src/ui/small_widgets/MyWidget.h \
    src/ui/small_widgets/SearchOverlay.h \
    src/ui/small_widgets/SlidingStackedWidget.h \
    src/ui/small_widgets/TagCloud.h \
    src/ui/small_widgets/TvShowTreeView.h \
    src/ui/tv_show/TvShowFilesWidget.h \
    src/ui/support/SupportDialog.h \
    src/scrapers/trailer/HdTrailers.h \
    src/scrapers/trailer/TrailerProvider.h \
    src/ui/tv_show/TvShowCommonWidgets.h \
    src/ui/tv_show/TvShowMultiScrapeDialog.h \
    src/ui/tv_show/TvShowSearch.h \
    src/ui/tv_show/TvShowWidget.h \
    src/ui/tv_show/TvShowWidgetEpisode.h \
    src/ui/tv_show/TvShowWidgetSeason.h \
    src/ui/tv_show/TvShowWidgetTvShow.h \
    src/ui/tv_show/TvTunesDialog.h \
    src/tv_shows/EpisodeMap.h \
    src/tv_shows/TvShowModel.h \
    src/tv_shows/TvShowProxyModel.h \
    src/tv_shows/model/TvShowModelItem.h \
    src/tv_shows/model/TvShowBaseModelItem.h \
    src/tv_shows/model/TvShowRootModelItem.h \
    src/tv_shows/model/EpisodeModelItem.h \
    src/tv_shows/model/SeasonModelItem.h \
    src/ui/tv_show/TvShowSearchWidget.h \
    src/tv_shows/TvShowUpdater.h \
    src/tv_shows/TvShowUtils.h \
    src/ui/media_centers/KodiSync.h \
    src/data/ImdbId.h \
    src/data/TmdbId.h \
    src/tv_shows/TvDbId.h \
    src/tv_shows/TvMazeId.h \
    src/tv_shows/EpisodeNumber.h \
    src/tv_shows/SeasonNumber.h \
    src/tv_shows/SeasonOrder.h \
    src/data/Certification.h \
    src/movies/MovieCrew.h \
    src/movies/MovieSet.h \
    src/scrapers/movie/MovieIdentifier.h

FORMS    += src/ui/main/MainWindow.ui \
    src/ui/concerts/ConcertFilesWidget.ui \
    src/ui/concerts/ConcertSearch.ui \
    src/ui/concerts/ConcertSearchWidget.ui \
    src/ui/concerts/ConcertStreamDetailsWidget.ui \
    src/ui/concerts/ConcertWidget.ui \
    src/ui/concerts/ConcertInfoWidget.ui \
    src/ui/export/CsvExportDialog.ui \
    src/ui/imports/DownloadsWidget.ui \
    src/ui/imports/ImportActions.ui \
    src/ui/imports/ImportDialog.ui \
    src/ui/imports/MakeMkvDialog.ui \
    src/ui/imports/UnpackButtons.ui \
    src/ui/export/ExportDialog.ui \
    src/globals/ImageDialog.ui \
    src/globals/ImagePreviewDialog.ui \
    src/globals/TrailerDialog.ui \
    src/ui/image/ImageWidget.ui \
    src/ui/main/AboutDialog.ui \
    src/ui/main/FileScannerDialog.ui \
    src/ui/main/Message.ui \
    src/ui/main/Navbar.ui \
    src/ui/movies/CertificationWidget.ui \
    src/ui/movies/GenreWidget.ui \
    src/ui/movies/MovieDuplicateItem.ui \
    src/ui/movies/MovieDuplicates.ui \
    src/ui/movies/MovieFilesWidget.ui \
    src/ui/movies/MovieMultiScrapeDialog.ui \
    src/ui/movies/MovieSearch.ui \
    src/ui/movies/MovieSearchWidget.ui \
    src/ui/movies/MovieWidget.ui \
    src/ui/music/MusicFilesWidget.ui \
    src/ui/music/MusicMultiScrapeDialog.ui \
    src/ui/music/MusicSearch.ui \
    src/ui/music/MusicSearchWidget.ui \
    src/ui/music/MusicWidget.ui \
    src/ui/music/MusicWidgetAlbum.ui \
    src/ui/music/MusicWidgetArtist.ui \
    src/ui/notifications/NotificationBox.ui \
    src/renamer/RenamerDialog.ui \
    src/renamer/RenamerPlaceholders.ui \
    src/ui/movie_sets/MovieListDialog.ui \
    src/ui/movie_sets/SetsWidget.ui \
    src/ui/settings/ExportTemplateWidget.ui \
    src/ui/settings/SettingsWindow.ui \
    src/ui/settings/ConcertSettingsWidget.ui \
    src/ui/settings/GlobalSettingsWidget.ui \
    src/ui/settings/ExportSettingsWidget.ui \
    src/ui/settings/ImportSettingsWidget.ui \
    src/ui/settings/KodiSettingsWidget.ui \
    src/ui/settings/MovieSettingsWidget.ui \
    src/ui/settings/MusicSettingsWidget.ui \
    src/ui/settings/NetworkSettingsWidget.ui \
    src/ui/settings/ScraperSettingsWidget.ui \
    src/ui/settings/TvScraperSettingsWidget.ui \
    src/ui/settings/CustomTvScraperSettingsWidget.ui \
    src/ui/settings/TvShowSettingsWidget.ui \
    src/ui/small_widgets/FilterWidget.ui \
    src/ui/small_widgets/ImageLabel.ui \
    src/ui/small_widgets/LoadingStreamDetails.ui \
    src/ui/small_widgets/MediaFlags.ui \
    src/ui/small_widgets/TagCloud.ui \
    src/ui/tv_show/TvShowFilesWidget.ui \
    src/ui/support/SupportDialog.ui \
    src/tv_shows/ItemWidgetShow.ui \
    src/ui/tv_show/TvShowMultiScrapeDialog.ui \
    src/ui/tv_show/TvShowSearch.ui \
    src/ui/tv_show/TvShowSearchWidget.ui \
    src/ui/tv_show/TvShowWidget.ui \
    src/ui/tv_show/TvShowWidgetEpisode.ui \
    src/ui/tv_show/TvShowWidgetSeason.ui \
    src/ui/tv_show/TvShowWidgetTvShow.ui \
    src/ui/tv_show/TvTunesDialog.ui \
    src/ui/media_centers/KodiSync.ui

RESOURCES += \
    data/MediaElch.qrc \
    ui.qrc

TRANSLATIONS += \
    data/i18n/MediaElch_bg.ts \
    data/i18n/MediaElch_cs_CZ.ts \
    data/i18n/MediaElch_da.ts \
    data/i18n/MediaElch_de.ts \
    data/i18n/MediaElch_en.ts \
    # See https://github.com/Komet/MediaElch/issues/1191#issuecomment-789104632
    # Locale resolution is stupid...
    # data/i18n/MediaElch_en_US.ts \
    data/i18n/MediaElch_es_ES.ts \
    data/i18n/MediaElch_fi.ts \
    data/i18n/MediaElch_fr.ts \
    data/i18n/MediaElch_it.ts \
    data/i18n/MediaElch_ja.ts \
    data/i18n/MediaElch_ko.ts \
    data/i18n/MediaElch_nl_NL.ts \
    data/i18n/MediaElch_no.ts \
    data/i18n/MediaElch_pl.ts \
    data/i18n/MediaElch_pt_BR.ts \
    data/i18n/MediaElch_pt_PT.ts \
    data/i18n/MediaElch_ru.ts \
    data/i18n/MediaElch_sv.ts \
    data/i18n/MediaElch_zh_CN.ts

sanitize {
    message(Sanitizer build)
    CONFIG += sanitizer sanitize_address

} else {
    message(Normal build)
}
