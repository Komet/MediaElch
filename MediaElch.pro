# Check Qt versions
lessThan(QT_MAJOR_VERSION, 5): error(Qt 4 is not supported!)
equals(QT_MAJOR_VERSION, 5) {
    lessThan(QT_MINOR_VERSION, 6): error(Qt 5.6 or higher is required!)
}
equals(QT_MAJOR_VERSION, 6) {
    lessThan(QT_MINOR_VERSION, 3): error("Qt 6.3 is required as 6.0 and 6.1 do not support QMultiMedia!")
}
contains(CONFIG, USE_EXTERN_QUAZIP) {
    DEFINES += EXTERN_QUAZIP
}

!contains(DEFINES, EXTERN_QUAZIP) {
    # using internal 3rd party QUAZIP
    DEFINES += QUAZIP_BUILD
    DEFINES += QUAZIP_STATIC # Required by Quazip to export symbols
    include($$PWD/third_party/quazip.pri)
    # For correct include paths
    INCLUDEPATH += third_party/quazip
}

TEMPLATE = app
TARGET = MediaElch
INCLUDEPATH += $$PWD/src
!contains(DEFINES, EXTERN_QUAZIP) {
    # using internal 3rd party QUAZIP
    INCLUDEPATH += $$PWD/third_party
}

QT += core gui network xml sql widgets multimedia multimediawidgets \
      concurrent opengl svg

equals(QT_MAJOR_VERSION, 6) {
    QT += core5compat
}

CONFIG += warn_on c++14
CONFIG += lrelease embed_translations
# LTO
CONFIG += ltcg

LIBS += -lz
contains(DEFINES, EXTERN_QUAZIP) {
    # using external quazip
    equals(QT_MAJOR_VERSION, 6) {
        warning("For Qt6 and the system's QuaZip, please use CMake!")
        LIBS += -lquazip6
    } else {
        LIBS += -lquazip5
    }
}

!contains(CONFIG, DISABLE_UPDATER) {
    DEFINES += MEDIAELCH_UPDATER
} else {
    message("Updater disabled")
}

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

# Usage as: CONFIG+=mold
mold {
    message("Using mold linker")
    QMAKE_LFLAGS += -fuse-ld=mold
}

# Usage as: CONFIG+=ubsan
ubsan {
    message("Using ubsan")
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

metaInfo.path = /usr/share/metainfo
metaInfo.files = data/desktop/com.kvibes.MediaElch.metainfo.xml
INSTALLS += metaInfo

ICON = MediaElch.icns
RC_FILE = MediaElch.rc
QMAKE_INFO_PLIST = MediaElch.plist

SOURCES += \
    src/data/Actor.cpp \
    src/data/AllMusicId.cpp \
    src/database/Database.cpp \
    src/database/DatabaseId.cpp \
    src/data/Certification.cpp \
    src/data/concert/ConcertController.cpp \
    src/data/concert/Concert.cpp \
    src/data/Filter.cpp \
    src/data/Image.cpp \
    src/data/ImdbId.cpp \
    src/data/Locale.cpp \
    src/data/movie/MovieController.cpp \
    src/data/movie/Movie.cpp \
    src/data/movie/MovieCrew.cpp \
    src/data/movie/MovieImages.cpp \
    src/data/movie/MovieSet.cpp \
    src/data/music/AlbumController.cpp \
    src/data/music/Album.cpp \
    src/data/music/ArtistController.cpp \
    src/data/music/Artist.cpp \
    src/data/MusicBrainzId.cpp \
    src/data/Poster.cpp \
    src/data/Rating.cpp \
    src/data/ResumeTime.cpp \
    src/data/Subtitle.cpp \
    src/data/TheAudioDbId.cpp \
    src/data/ThumbnailDimensions.cpp \
    src/data/TmdbId.cpp \
    src/data/TvDbId.cpp \
    src/data/TvMazeId.cpp \
    src/data/tv_show/EpisodeMap.cpp \
    src/data/tv_show/EpisodeNumber.cpp \
    src/data/tv_show/SeasonNumber.cpp \
    src/data/tv_show/SeasonOrder.cpp \
    src/data/tv_show/TvShow.cpp \
    src/data/tv_show/TvShowEpisode.cpp \
    src/data/WikidataId.cpp \
    src/export/CsvExport.cpp \
    src/export/ExportTemplate.cpp \
    src/export/ExportTemplateLoader.cpp \
    src/export/MediaExport.cpp \
    src/export/SimpleEngine.cpp \
    src/export/TableWriter.cpp \
    src/file_search/ConcertFileSearcher.cpp \
    src/file_search/MovieFilesOrganizer.cpp \
    src/file_search/movie/MovieDirectorySearcher.cpp \
    src/file_search/movie/MovieDirScan.cpp \
    src/file_search/movie/MovieFileSearcher.cpp \
    src/file_search/MusicFileSearcher.cpp \
    src/file_search/TvShowFileSearcher.cpp \
    src/globals/Globals.cpp \
    src/globals/Helper.cpp \
    src/globals/Manager.cpp \
    src/globals/MediaDirectory.cpp \
    src/globals/MessageIds.cpp \
    src/globals/Module.cpp \
    src/globals/VersionInfo.cpp \
    src/import/DownloadFileSearcher.cpp \
    src/import/Extractor.cpp \
    src/import/FileWorker.cpp \
    src/import/MakeMkvCon.cpp \
    src/import/MyFile.cpp \
    src/log/Log.cpp \
    src/main.cpp \
    src/media/AsyncImage.cpp \
    src/media_center/kodi/AlbumXmlReader.cpp \
    src/media_center/kodi/AlbumXmlWriter.cpp \
    src/media_center/kodi/ArtistXmlReader.cpp \
    src/media_center/kodi/ArtistXmlWriter.cpp \
    src/media_center/kodi/ConcertXmlReader.cpp \
    src/media_center/kodi/ConcertXmlWriter.cpp \
    src/media_center/kodi/EpisodeXmlReader.cpp \
    src/media_center/kodi/EpisodeXmlWriter.cpp \
    src/media_center/kodi/KodiXmlWriter.cpp \
    src/media_center/kodi/MovieXmlReader.cpp \
    src/media_center/kodi/MovieXmlWriter.cpp \
    src/media_center/kodi/TvShowXmlReader.cpp \
    src/media_center/kodi/TvShowXmlWriter.cpp \
    src/media_center/KodiVersion.cpp \
    src/media_center/KodiXml.cpp \
    src/media_center/MediaCenterInterface.cpp \
    src/media/FileFilter.cpp \
    src/media/FilenameUtils.cpp \
    src/media/ImageCache.cpp \
    src/media/ImageCapture.cpp \
    src/media/ImageUtils.cpp \
    src/media/MediaInfoFile.cpp \
    src/media/NameFormatter.cpp \
    src/media/Path.cpp \
    src/media/StreamDetails.cpp \
    src/model/ActorModel.cpp \
    src/model/ConcertModel.cpp \
    src/model/ConcertProxyModel.cpp \
    src/model/ImageModel.cpp \
    src/model/ImageProxyModel.cpp \
    src/model/MediaStatusColumn.cpp \
    src/model/MovieModel.cpp \
    src/model/MovieProxyModel.cpp \
    src/model/music/MusicModel.cpp \
    src/model/music/MusicModelItem.cpp \
    src/model/music/MusicModelRoles.cpp \
    src/model/music/MusicProxyModel.cpp \
    src/model/RatingModel.cpp \
    src/model/tv_show/EpisodeModelItem.cpp \
    src/model/TvShowModel.cpp \
    src/model/TvShowProxyModel.cpp \
    src/model/tv_show/SeasonModelItem.cpp \
    src/model/tv_show/TvShowBaseModelItem.cpp \
    src/model/tv_show/TvShowModelItem.cpp \
    src/model/tv_show/TvShowRootModelItem.cpp \
    src/model/tv_show/TvShowUtils.cpp \
    src/network/DownloadManager.cpp \
    src/network/DownloadManagerElement.cpp \
    src/network/HttpStatusCodes.cpp \
    src/network/NetworkManager.cpp \
    src/network/NetworkReplyWatcher.cpp \
    src/network/NetworkRequest.cpp \
    src/network/WebsiteCache.cpp \
    src/renamer/ConcertRenamer.cpp \
    src/renamer/EpisodeRenamer.cpp \
    src/renamer/MovieRenamer.cpp \
    src/renamer/Renamer.cpp \
    src/scrapers/concert/ConcertIdentifier.cpp \
    src/scrapers/concert/ConcertScraper.cpp \
    src/scrapers/concert/ConcertSearchJob.cpp \
    src/scrapers/concert/tmdb/TmdbConcertConfiguration.cpp \
    src/scrapers/concert/tmdb/TmdbConcert.cpp \
    src/scrapers/concert/tmdb/TmdbConcertSearchJob.cpp \
    src/scrapers/image/FanartTvConfiguration.cpp \
    src/scrapers/image/FanartTv.cpp \
    src/scrapers/image/FanartTvMusicArtists.cpp \
    src/scrapers/image/FanartTvMusic.cpp \
    src/scrapers/image/ImageProvider.cpp \
    src/scrapers/image/TheTvDbImages.cpp \
    src/scrapers/image/TmdbImages.cpp \
    src/scrapers/imdb/ImdbApi.cpp \
    src/scrapers/imdb/ImdbReferencePage.cpp \
    src/scrapers/movie/adultdvdempire/AdultDvdEmpireApi.cpp \
    src/scrapers/movie/adultdvdempire/AdultDvdEmpire.cpp \
    src/scrapers/movie/adultdvdempire/AdultDvdEmpireScrapeJob.cpp \
    src/scrapers/movie/adultdvdempire/AdultDvdEmpireSearchJob.cpp \
    src/scrapers/movie/aebn/AebnApi.cpp \
    src/scrapers/movie/aebn/AebnConfiguration.cpp \
    src/scrapers/movie/aebn/AEBN.cpp \
    src/scrapers/movie/aebn/AebnScrapeJob.cpp \
    src/scrapers/movie/aebn/AebnSearchJob.cpp \
    src/scrapers/movie/custom/CustomMovieScrapeJob.cpp \
    src/scrapers/movie/custom/CustomMovieScraperConfiguration.cpp \
    src/scrapers/movie/custom/CustomMovieScraper.cpp \
    src/scrapers/movie/hotmovies/HotMoviesApi.cpp \
    src/scrapers/movie/hotmovies/HotMovies.cpp \
    src/scrapers/movie/hotmovies/HotMoviesScrapeJob.cpp \
    src/scrapers/movie/hotmovies/HotMoviesSearchJob.cpp \
    src/scrapers/movie/imdb/ImdbMovieConfiguration.cpp \
    src/scrapers/movie/imdb/ImdbMovie.cpp \
    src/scrapers/movie/imdb/ImdbMovieScrapeJob.cpp \
    src/scrapers/movie/imdb/ImdbMovieSearchJob.cpp \
    src/scrapers/movie/MovieIdentifier.cpp \
    src/scrapers/movie/MovieMerger.cpp \
    src/scrapers/movie/MovieScrapeJob.cpp \
    src/scrapers/movie/MovieScraper.cpp \
    src/scrapers/movie/MovieSearchJob.cpp \
    src/scrapers/movie/tmdb/TmdbMovieConfiguration.cpp \
    src/scrapers/movie/tmdb/TmdbMovie.cpp \
    src/scrapers/movie/tmdb/TmdbMovieScrapeJob.cpp \
    src/scrapers/movie/tmdb/TmdbMovieSearchJob.cpp \
    src/scrapers/movie/videobuster/VideoBusterApi.cpp \
    src/scrapers/movie/videobuster/VideoBuster.cpp \
    src/scrapers/movie/videobuster/VideoBusterScrapeJob.cpp \
    src/scrapers/movie/videobuster/VideoBusterSearchJob.cpp \
    src/scrapers/music/AllMusic.cpp \
    src/scrapers/music/Discogs.cpp \
    src/scrapers/music/MusicBrainz.cpp \
    src/scrapers/music/MusicMerger.cpp \
    src/scrapers/music/MusicScraper.cpp \
    src/scrapers/music/TheAudioDb.cpp \
    src/scrapers/music/TvTunes.cpp \
    src/scrapers/music/UniversalMusicConfiguration.cpp \
    src/scrapers/music/UniversalMusicScraper.cpp \
    src/scrapers/ScraperConfiguration.cpp \
    src/scrapers/ScraperError.cpp \
    src/scrapers/ScraperInfos.cpp \
    src/scrapers/ScraperInterface.cpp \
    src/scrapers/ScraperResult.cpp \
    src/scrapers/ScraperUtils.cpp \
    src/scrapers/tmdb/TmdbApi.cpp \
    src/scrapers/trailer/HdTrailers.cpp \
    src/scrapers/trailer/TrailerProvider.cpp \
    src/scrapers/trailer/TrailerResult.cpp \
    src/scrapers/tv_show/custom/CustomEpisodeScrapeJob.cpp \
    src/scrapers/tv_show/custom/CustomSeasonScrapeJob.cpp \
    src/scrapers/tv_show/custom/CustomShowScrapeJob.cpp \
    src/scrapers/tv_show/custom/CustomTvScraperConfiguration.cpp \
    src/scrapers/tv_show/custom/CustomTvScraper.cpp \
    src/scrapers/tv_show/empty/EmptyTvScraper.cpp \
    src/scrapers/tv_show/EpisodeIdentifier.cpp \
    src/scrapers/tv_show/EpisodeScrapeJob.cpp \
    src/scrapers/tv_show/fernsehserien_de/FernsehserienDeConfiguration.cpp \
    src/scrapers/tv_show/fernsehserien_de/FernsehserienDe.cpp \
    src/scrapers/tv_show/imdb/ImdbTvConfiguration.cpp \
    src/scrapers/tv_show/imdb/ImdbTv.cpp \
    src/scrapers/tv_show/imdb/ImdbTvEpisodeParser.cpp \
    src/scrapers/tv_show/imdb/ImdbTvEpisodeScrapeJob.cpp \
    src/scrapers/tv_show/imdb/ImdbTvSeasonParser.cpp \
    src/scrapers/tv_show/imdb/ImdbTvSeasonScrapeJob.cpp \
    src/scrapers/tv_show/imdb/ImdbTvShowParser.cpp \
    src/scrapers/tv_show/imdb/ImdbTvShowScrapeJob.cpp \
    src/scrapers/tv_show/imdb/ImdbTvShowSearchJob.cpp \
    src/scrapers/tv_show/SeasonScrapeJob.cpp \
    src/scrapers/tv_show/ShowIdentifier.cpp \
    src/scrapers/tv_show/ShowMerger.cpp \
    src/scrapers/tv_show/ShowScrapeJob.cpp \
    src/scrapers/tv_show/ShowSearchJob.cpp \
    src/scrapers/tv_show/thetvdb/TheTvDbApi.cpp \
    src/scrapers/tv_show/thetvdb/TheTvDbConfiguration.cpp \
    src/scrapers/tv_show/thetvdb/TheTvDb.cpp \
    src/scrapers/tv_show/thetvdb/TheTvDbEpisodeParser.cpp \
    src/scrapers/tv_show/thetvdb/TheTvDbEpisodeScrapeJob.cpp \
    src/scrapers/tv_show/thetvdb/TheTvDbEpisodesParser.cpp \
    src/scrapers/tv_show/thetvdb/TheTvDbSeasonScrapeJob.cpp \
    src/scrapers/tv_show/thetvdb/TheTvDbShowParser.cpp \
    src/scrapers/tv_show/thetvdb/TheTvDbShowScrapeJob.cpp \
    src/scrapers/tv_show/thetvdb/TheTvDbShowSearchJob.cpp \
    src/scrapers/tv_show/tmdb/TmdbTvConfiguration.cpp \
    src/scrapers/tv_show/tmdb/TmdbTv.cpp \
    src/scrapers/tv_show/tmdb/TmdbTvEpisodeParser.cpp \
    src/scrapers/tv_show/tmdb/TmdbTvEpisodeScrapeJob.cpp \
    src/scrapers/tv_show/tmdb/TmdbTvSeasonParser.cpp \
    src/scrapers/tv_show/tmdb/TmdbTvSeasonScrapeJob.cpp \
    src/scrapers/tv_show/tmdb/TmdbTvShowParser.cpp \
    src/scrapers/tv_show/tmdb/TmdbTvShowScrapeJob.cpp \
    src/scrapers/tv_show/tmdb/TmdbTvShowSearchJob.cpp \
    src/scrapers/tv_show/tvmaze/TvMazeApi.cpp \
    src/scrapers/tv_show/tvmaze/TvMazeConfiguration.cpp \
    src/scrapers/tv_show/tvmaze/TvMaze.cpp \
    src/scrapers/tv_show/tvmaze/TvMazeEpisodeParser.cpp \
    src/scrapers/tv_show/tvmaze/TvMazeEpisodeScrapeJob.cpp \
    src/scrapers/tv_show/tvmaze/TvMazeSeasonScrapeJob.cpp \
    src/scrapers/tv_show/tvmaze/TvMazeShowParser.cpp \
    src/scrapers/tv_show/tvmaze/TvMazeShowScrapeJob.cpp \
    src/scrapers/tv_show/tvmaze/TvMazeShowSearchJob.cpp \
    src/scrapers/tv_show/TvScraper.cpp \
    src/scrapers/TvShowUpdater.cpp \
    src/settings/AdvancedSettings.cpp \
    src/settings/AdvancedSettingsXmlReader.cpp \
    src/settings/DataFile.cpp \
    src/settings/DirectorySettings.cpp \
    src/settings/ImportSettings.cpp \
    src/settings/KodiSettings.cpp \
    src/settings/NetworkSettings.cpp \
    src/settings/Settings.cpp \
    src/ui/concerts/ConcertFilesWidget.cpp \
    src/ui/concerts/ConcertInfoWidget.cpp \
    src/ui/concerts/ConcertSearch.cpp \
    src/ui/concerts/ConcertSearchWidget.cpp \
    src/ui/concerts/ConcertStreamDetailsWidget.cpp \
    src/ui/concerts/ConcertWidget.cpp \
    src/ui/export/csv_export/CsvExportConfiguration.cpp \
    src/ui/export/csv_export/CsvExportDialog.cpp \
    src/ui/export/csv_export/CsvExportModule.cpp \
    src/ui/export/ExportDialog.cpp \
    src/ui/image/ImageDialog.cpp \
    src/ui/image/ImagePreviewDialog.cpp \
    src/ui/import/DownloadsWidget.cpp \
    src/ui/import/ImportActions.cpp \
    src/ui/import/ImportDialog.cpp \
    src/ui/import/MakeMkvDialog.cpp \
    src/ui/import/UnpackButtons.cpp \
    src/ui/main/AboutDialog.cpp \
    src/ui/main/FileScannerDialog.cpp \
    src/ui/main/MainWindow.cpp \
    src/ui/main/Message.cpp \
    src/ui/main/MyIconFont.cpp \
    src/ui/main/Navbar.cpp \
    src/ui/main/QuickOpen.cpp \
    src/ui/main/Update.cpp \
    src/ui/media_center/KodiSync.cpp \
    src/ui/movies/CertificationWidget.cpp \
    src/ui/movie_sets/MovieListDialog.cpp \
    src/ui/movie_sets/SetsWidget.cpp \
    src/ui/movies/GenreWidget.cpp \
    src/ui/movies/MovieDuplicateItem.cpp \
    src/ui/movies/MovieDuplicates.cpp \
    src/ui/movies/MovieFilesWidget.cpp \
    src/ui/movies/MovieMultiScrapeDialog.cpp \
    src/ui/movies/MoviePreviewAdapter.cpp \
    src/ui/movies/MovieSearch.cpp \
    src/ui/movies/MovieSearchWidget.cpp \
    src/ui/movies/MovieWidget.cpp \
    src/ui/music/MusicFilesWidget.cpp \
    src/ui/music/MusicMultiScrapeDialog.cpp \
    src/ui/music/MusicSearch.cpp \
    src/ui/music/MusicSearchWidget.cpp \
    src/ui/music/MusicWidgetAlbum.cpp \
    src/ui/music/MusicWidgetArtist.cpp \
    src/ui/music/MusicWidget.cpp \
    src/ui/notifications/NotificationBox.cpp \
    src/ui/notifications/Notificator.cpp \
    src/ui/renamer/RenamerDialog.cpp \
    src/ui/renamer/RenamerPlaceholders.cpp \
    src/ui/scrapers/concert/TmdbConcertConfigurationView.cpp \
    src/ui/scrapers/image/FanartTvConfigurationView.cpp \
    src/ui/scrapers/movie/AebnConfigurationView.cpp \
    src/ui/scrapers/movie/ImdbMovieConfigurationView.cpp \
    src/ui/scrapers/movie/TmdbMovieConfigurationView.cpp \
    src/ui/scrapers/music/UniversalMusicConfigurationView.cpp \
    src/ui/scrapers/ScraperManager.cpp \
    src/ui/scrapers/tv_show/FernsehserienDeConfigurationView.cpp \
    src/ui/scrapers/tv_show/ImdbTvConfigurationView.cpp \
    src/ui/scrapers/tv_show/TheTvDbConfigurationView.cpp \
    src/ui/scrapers/tv_show/TmdbTvConfigurationView.cpp \
    src/ui/scrapers/tv_show/TvMazeConfigurationView.cpp \
    src/ui/settings/ConcertScraperInfoWidget.cpp \
    src/ui/settings/ConcertSettingsWidget.cpp \
    src/ui/settings/CustomTvScraperSettingsWidget.cpp \
    src/ui/settings/ExportSettingsWidget.cpp \
    src/ui/settings/ExportTemplateWidget.cpp \
    src/ui/settings/GlobalSettingsWidget.cpp \
    src/ui/settings/ImageProviderInfoWidget.cpp \
    src/ui/settings/ImportSettingsWidget.cpp \
    src/ui/settings/KodiSettingsWidget.cpp \
    src/ui/settings/MovieScraperInfoWidget.cpp \
    src/ui/settings/MovieSettingsWidget.cpp \
    src/ui/settings/MusicScraperInfoWidget.cpp \
    src/ui/settings/MusicSettingsWidget.cpp \
    src/ui/settings/NetworkSettingsWidget.cpp \
    src/ui/settings/ScraperSettingsTable.cpp \
    src/ui/settings/ScraperSettingsWidget.cpp \
    src/ui/settings/SettingsWindow.cpp \
    src/ui/settings/TvScraperInfoWidget.cpp \
    src/ui/settings/TvShowSettingsWidget.cpp \
    src/ui/small_widgets/ActorsWidget.cpp \
    src/ui/small_widgets/AlphabeticalList.cpp \
    src/ui/small_widgets/Badge.cpp \
    src/ui/small_widgets/ClosableImage.cpp \
    src/ui/small_widgets/ComboDelegate.cpp \
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
    src/ui/small_widgets/PlaceholderLineEdit.cpp \
    src/ui/small_widgets/RatingSourceDelegate.cpp \
    src/ui/small_widgets/RatingsWidget.cpp \
    src/ui/small_widgets/ScrapePreview.cpp \
    src/ui/small_widgets/SearchOverlay.cpp \
    src/ui/small_widgets/SlidingStackedWidget.cpp \
    src/ui/small_widgets/SpinBoxDelegate.cpp \
    src/ui/small_widgets/StereoModeComboBox.cpp \
    src/ui/small_widgets/TagCloud.cpp \
    src/ui/small_widgets/TvShowTreeView.cpp \
    src/ui/small_widgets/WebImageLabel.cpp \
    src/ui/support/SupportDialog.cpp \
    src/ui/trailer/TrailerDialog.cpp \
    src/ui/tv_show/search_dialog/TvShowPreviewAdapter.cpp \
    src/ui/tv_show/TvShowCommonWidgets.cpp \
    src/ui/tv_show/TvShowFilesWidget.cpp \
    src/ui/tv_show/TvShowMultiScrapeDialog.cpp \
    src/ui/tv_show/TvShowSearch.cpp \
    src/ui/tv_show/TvShowSearchWidget.cpp \
    src/ui/tv_show/TvShowWidget.cpp \
    src/ui/tv_show/TvShowWidgetEpisode.cpp \
    src/ui/tv_show/TvShowWidgetSeason.cpp \
    src/ui/tv_show/TvShowWidgetTvShow.cpp \
    src/ui/tv_show/TvTunesDialog.cpp \
    src/ui/UiUtils.cpp \
    src/utils/Containers.cpp \
    src/utils/Math.cpp \
    src/utils/Meta.cpp \
    src/utils/Random.cpp \
    src/utils/Time.cpp \
    src/workers/Job.cpp

HEADERS += Version.h \
    src/data/Actor.h \
    src/data/AllMusicId.h \
    src/database/Database.h \
    src/database/DatabaseId.h \
    src/data/Certification.h \
    src/data/concert/ConcertController.h \
    src/data/concert/Concert.h \
    src/data/Filter.h \
    src/data/Image.h \
    src/data/ImdbId.h \
    src/data/Locale.h \
    src/data/movie/MovieController.h \
    src/data/movie/MovieCrew.h \
    src/data/movie/Movie.h \
    src/data/movie/MovieImages.h \
    src/data/movie/MovieSet.h \
    src/data/music/AlbumController.h \
    src/data/music/Album.h \
    src/data/music/ArtistController.h \
    src/data/music/Artist.h \
    src/data/MusicBrainzId.h \
    src/data/Poster.h \
    src/data/Rating.h \
    src/data/ResumeTime.h \
    src/data/Subtitle.h \
    src/data/TheAudioDbId.h \
    src/data/ThumbnailDimensions.h \
    src/data/TmdbId.h \
    src/data/TvDbId.h \
    src/data/TvMazeId.h \
    src/data/tv_show/EpisodeMap.h \
    src/data/tv_show/EpisodeNumber.h \
    src/data/tv_show/SeasonNumber.h \
    src/data/tv_show/SeasonOrder.h \
    src/data/tv_show/TvShowEpisode.h \
    src/data/tv_show/TvShow.h \
    src/data/WikidataId.h \
    src/export/CsvExport.h \
    src/export/ExportTemplate.h \
    src/export/ExportTemplateLoader.h \
    src/export/MediaExport.h \
    src/export/SimpleEngine.h \
    src/export/TableWriter.h \
    src/file_search/ConcertFileSearcher.h \
    src/file_search/MovieFilesOrganizer.h \
    src/file_search/movie/MovieDirectorySearcher.h \
    src/file_search/movie/MovieDirScan.h \
    src/file_search/movie/MovieFileSearcher.h \
    src/file_search/MusicFileSearcher.h \
    src/file_search/TvShowFileSearcher.h \
    src/globals/Globals.h \
    src/globals/Helper.h \
    src/globals/LocaleStringCompare.h \
    src/globals/Manager.h \
    src/globals/MediaDirectory.h \
    src/globals/MessageIds.h \
    src/globals/Module.h \
    src/globals/VersionInfo.h \
    src/import/DownloadFileSearcher.h \
    src/import/Extractor.h \
    src/import/FileWorker.h \
    src/import/MakeMkvCon.h \
    src/import/MyFile.h \
    src/log/Log.h \
    src/media/AsyncImage.h \
    src/media_center/kodi/AlbumXmlReader.h \
    src/media_center/kodi/AlbumXmlWriter.h \
    src/media_center/kodi/ArtistXmlReader.h \
    src/media_center/kodi/ArtistXmlWriter.h \
    src/media_center/kodi/ConcertXmlReader.h \
    src/media_center/kodi/ConcertXmlWriter.h \
    src/media_center/kodi/EpisodeXmlReader.h \
    src/media_center/kodi/EpisodeXmlWriter.h \
    src/media_center/kodi/KodiXmlWriter.h \
    src/media_center/kodi/MovieXmlReader.h \
    src/media_center/kodi/MovieXmlWriter.h \
    src/media_center/kodi/TvShowXmlReader.h \
    src/media_center/kodi/TvShowXmlWriter.h \
    src/media_center/KodiVersion.h \
    src/media_center/KodiXml.h \
    src/media_center/MediaCenterInterface.h \
    src/media/FileFilter.h \
    src/media/FilenameUtils.h \
    src/media/ImageCache.h \
    src/media/ImageCapture.h \
    src/media/ImageUtils.h \
    src/media/MediaInfoFile.h \
    src/media/NameFormatter.h \
    src/media/Path.h \
    src/media/StreamDetails.h \
    src/model/ActorModel.h \
    src/model/ConcertModel.h \
    src/model/ConcertProxyModel.h \
    src/model/ImageModel.h \
    src/model/ImageProxyModel.h \
    src/model/MediaStatusColumn.h \
    src/model/MovieModel.h \
    src/model/MovieProxyModel.h \
    src/model/music/MusicModel.h \
    src/model/music/MusicModelItem.h \
    src/model/music/MusicModelRoles.h \
    src/model/music/MusicProxyModel.h \
    src/model/RatingModel.h \
    src/model/tv_show/EpisodeModelItem.h \
    src/model/TvShowModel.h \
    src/model/TvShowProxyModel.h \
    src/model/tv_show/SeasonModelItem.h \
    src/model/tv_show/TvShowBaseModelItem.h \
    src/model/tv_show/TvShowModelItem.h \
    src/model/tv_show/TvShowRootModelItem.h \
    src/model/tv_show/TvShowUtils.h \
    src/network/DownloadManagerElement.h \
    src/network/DownloadManager.h \
    src/network/HttpStatusCodes.h \
    src/network/NetworkManager.h \
    src/network/NetworkReplyWatcher.h \
    src/network/NetworkRequest.h \
    src/network/WebsiteCache.h \
    src/renamer/ConcertRenamer.h \
    src/renamer/EpisodeRenamer.h \
    src/renamer/MovieRenamer.h \
    src/renamer/Renamer.h \
    src/scrapers/concert/ConcertIdentifier.h \
    src/scrapers/concert/ConcertScraper.h \
    src/scrapers/concert/ConcertSearchJob.h \
    src/scrapers/concert/tmdb/TmdbConcertConfiguration.h \
    src/scrapers/concert/tmdb/TmdbConcert.h \
    src/scrapers/concert/tmdb/TmdbConcertSearchJob.h \
    src/scrapers/image/FanartTvConfiguration.h \
    src/scrapers/image/FanartTv.h \
    src/scrapers/image/FanartTvMusicArtists.h \
    src/scrapers/image/FanartTvMusic.h \
    src/scrapers/image/ImageProvider.h \
    src/scrapers/image/TheTvDbImages.h \
    src/scrapers/image/TmdbImages.h \
    src/scrapers/imdb/ImdbApi.h \
    src/scrapers/imdb/ImdbReferencePage.h \
    src/scrapers/movie/adultdvdempire/AdultDvdEmpireApi.h \
    src/scrapers/movie/adultdvdempire/AdultDvdEmpire.h \
    src/scrapers/movie/adultdvdempire/AdultDvdEmpireScrapeJob.h \
    src/scrapers/movie/adultdvdempire/AdultDvdEmpireSearchJob.h \
    src/scrapers/movie/aebn/AebnApi.h \
    src/scrapers/movie/aebn/AebnConfiguration.h \
    src/scrapers/movie/aebn/AEBN.h \
    src/scrapers/movie/aebn/AebnScrapeJob.h \
    src/scrapers/movie/aebn/AebnSearchJob.h \
    src/scrapers/movie/custom/CustomMovieScrapeJob.h \
    src/scrapers/movie/custom/CustomMovieScraperConfiguration.h \
    src/scrapers/movie/custom/CustomMovieScraper.h \
    src/scrapers/movie/hotmovies/HotMoviesApi.h \
    src/scrapers/movie/hotmovies/HotMovies.h \
    src/scrapers/movie/hotmovies/HotMoviesScrapeJob.h \
    src/scrapers/movie/hotmovies/HotMoviesSearchJob.h \
    src/scrapers/movie/imdb/ImdbMovieConfiguration.h \
    src/scrapers/movie/imdb/ImdbMovie.h \
    src/scrapers/movie/imdb/ImdbMovieScrapeJob.h \
    src/scrapers/movie/imdb/ImdbMovieSearchJob.h \
    src/scrapers/movie/MovieIdentifier.h \
    src/scrapers/movie/MovieMerger.h \
    src/scrapers/movie/MovieScrapeJob.h \
    src/scrapers/movie/MovieScraper.h \
    src/scrapers/movie/MovieSearchJob.h \
    src/scrapers/movie/tmdb/TmdbMovieConfiguration.h \
    src/scrapers/movie/tmdb/TmdbMovie.h \
    src/scrapers/movie/tmdb/TmdbMovieScrapeJob.h \
    src/scrapers/movie/tmdb/TmdbMovieSearchJob.h \
    src/scrapers/movie/videobuster/VideoBusterApi.h \
    src/scrapers/movie/videobuster/VideoBuster.h \
    src/scrapers/movie/videobuster/VideoBusterScrapeJob.h \
    src/scrapers/movie/videobuster/VideoBusterSearchJob.h \
    src/scrapers/music/AllMusic.h \
    src/scrapers/music/Discogs.h \
    src/scrapers/music/MusicBrainz.h \
    src/scrapers/music/MusicMerger.h \
    src/scrapers/music/MusicScraper.h \
    src/scrapers/music/TheAudioDb.h \
    src/scrapers/music/TvTunes.h \
    src/scrapers/music/UniversalMusicConfiguration.h \
    src/scrapers/music/UniversalMusicScraper.h \
    src/scrapers/ScraperConfiguration.h \
    src/scrapers/ScraperError.h \
    src/scrapers/ScraperInfos.h \
    src/scrapers/ScraperInterface.h \
    src/scrapers/ScraperResult.h \
    src/scrapers/ScraperUtils.h \
    src/scrapers/tmdb/TmdbApi.h \
    src/scrapers/trailer/HdTrailers.h \
    src/scrapers/trailer/TrailerProvider.h \
    src/scrapers/trailer/TrailerResult.h \
    src/scrapers/tv_show/custom/CustomEpisodeScrapeJob.h \
    src/scrapers/tv_show/custom/CustomSeasonScrapeJob.h \
    src/scrapers/tv_show/custom/CustomShowScrapeJob.h \
    src/scrapers/tv_show/custom/CustomTvScraperConfiguration.h \
    src/scrapers/tv_show/custom/CustomTvScraper.h \
    src/scrapers/tv_show/empty/EmptyTvScraper.h \
    src/scrapers/tv_show/EpisodeIdentifier.h \
    src/scrapers/tv_show/EpisodeScrapeJob.h \
    src/scrapers/tv_show/fernsehserien_de/FernsehserienDeConfiguration.h \
    src/scrapers/tv_show/fernsehserien_de/FernsehserienDe.h \
    src/scrapers/tv_show/imdb/ImdbTvConfiguration.h \
    src/scrapers/tv_show/imdb/ImdbTvEpisodeParser.h \
    src/scrapers/tv_show/imdb/ImdbTvEpisodeScrapeJob.h \
    src/scrapers/tv_show/imdb/ImdbTv.h \
    src/scrapers/tv_show/imdb/ImdbTvSeasonParser.h \
    src/scrapers/tv_show/imdb/ImdbTvSeasonScrapeJob.h \
    src/scrapers/tv_show/imdb/ImdbTvShowParser.h \
    src/scrapers/tv_show/imdb/ImdbTvShowScrapeJob.h \
    src/scrapers/tv_show/imdb/ImdbTvShowSearchJob.h \
    src/scrapers/tv_show/SeasonScrapeJob.h \
    src/scrapers/tv_show/ShowIdentifier.h \
    src/scrapers/tv_show/ShowMerger.h \
    src/scrapers/tv_show/ShowScrapeJob.h \
    src/scrapers/tv_show/ShowSearchJob.h \
    src/scrapers/tv_show/thetvdb/TheTvDbApi.h \
    src/scrapers/tv_show/thetvdb/TheTvDbConfiguration.h \
    src/scrapers/tv_show/thetvdb/TheTvDbEpisodeParser.h \
    src/scrapers/tv_show/thetvdb/TheTvDbEpisodeScrapeJob.h \
    src/scrapers/tv_show/thetvdb/TheTvDbEpisodesParser.h \
    src/scrapers/tv_show/thetvdb/TheTvDb.h \
    src/scrapers/tv_show/thetvdb/TheTvDbSeasonScrapeJob.h \
    src/scrapers/tv_show/thetvdb/TheTvDbShowParser.h \
    src/scrapers/tv_show/thetvdb/TheTvDbShowScrapeJob.h \
    src/scrapers/tv_show/thetvdb/TheTvDbShowSearchJob.h \
    src/scrapers/tv_show/tmdb/TmdbTvConfiguration.h \
    src/scrapers/tv_show/tmdb/TmdbTvEpisodeParser.h \
    src/scrapers/tv_show/tmdb/TmdbTvEpisodeScrapeJob.h \
    src/scrapers/tv_show/tmdb/TmdbTv.h \
    src/scrapers/tv_show/tmdb/TmdbTvSeasonParser.h \
    src/scrapers/tv_show/tmdb/TmdbTvSeasonScrapeJob.h \
    src/scrapers/tv_show/tmdb/TmdbTvShowParser.h \
    src/scrapers/tv_show/tmdb/TmdbTvShowScrapeJob.h \
    src/scrapers/tv_show/tmdb/TmdbTvShowSearchJob.h \
    src/scrapers/tv_show/tvmaze/TvMazeApi.h \
    src/scrapers/tv_show/tvmaze/TvMazeConfiguration.h \
    src/scrapers/tv_show/tvmaze/TvMazeEpisodeParser.h \
    src/scrapers/tv_show/tvmaze/TvMazeEpisodeScrapeJob.h \
    src/scrapers/tv_show/tvmaze/TvMaze.h \
    src/scrapers/tv_show/tvmaze/TvMazeSeasonScrapeJob.h \
    src/scrapers/tv_show/tvmaze/TvMazeShowParser.h \
    src/scrapers/tv_show/tvmaze/TvMazeShowScrapeJob.h \
    src/scrapers/tv_show/tvmaze/TvMazeShowSearchJob.h \
    src/scrapers/tv_show/TvScraper.h \
    src/scrapers/TvShowUpdater.h \
    src/settings/AdvancedSettings.h \
    src/settings/AdvancedSettingsXmlReader.h \
    src/settings/DataFile.h \
    src/settings/DirectorySettings.h \
    src/settings/ImportSettings.h \
    src/settings/KodiSettings.h \
    src/settings/NetworkSettings.h \
    src/settings/Settings.h \
    src/ui/concerts/ConcertFilesWidget.h \
    src/ui/concerts/ConcertInfoWidget.h \
    src/ui/concerts/ConcertSearch.h \
    src/ui/concerts/ConcertSearchWidget.h \
    src/ui/concerts/ConcertStreamDetailsWidget.h \
    src/ui/concerts/ConcertWidget.h \
    src/ui/export/csv_export/CsvExportConfiguration.h \
    src/ui/export/csv_export/CsvExportDialog.h \
    src/ui/export/csv_export/CsvExportModule.h \
    src/ui/export/ExportDialog.h \
    src/ui/image/ImageDialog.h \
    src/ui/image/ImagePreviewDialog.h \
    src/ui/import/DownloadsWidget.h \
    src/ui/import/ImportActions.h \
    src/ui/import/ImportDialog.h \
    src/ui/import/MakeMkvDialog.h \
    src/ui/import/UnpackButtons.h \
    src/ui/MacUiUtilities.h \
    src/ui/main/AboutDialog.h \
    src/ui/main/FileScannerDialog.h \
    src/ui/main/MainWindow.h \
    src/ui/main/Message.h \
    src/ui/main/MyIconFont.h \
    src/ui/main/Navbar.h \
    src/ui/main/QuickOpen.h \
    src/ui/main/Update.h \
    src/ui/media_center/KodiSync.h \
    src/ui/movies/CertificationWidget.h \
    src/ui/movie_sets/MovieListDialog.h \
    src/ui/movie_sets/SetsWidget.h \
    src/ui/movies/GenreWidget.h \
    src/ui/movies/MovieDuplicateItem.h \
    src/ui/movies/MovieDuplicates.h \
    src/ui/movies/MovieFilesWidget.h \
    src/ui/movies/MovieMultiScrapeDialog.h \
    src/ui/movies/MoviePreviewAdapter.h \
    src/ui/movies/MovieSearch.h \
    src/ui/movies/MovieSearchWidget.h \
    src/ui/movies/MovieWidget.h \
    src/ui/music/MusicFilesWidget.h \
    src/ui/music/MusicMultiScrapeDialog.h \
    src/ui/music/MusicSearch.h \
    src/ui/music/MusicSearchWidget.h \
    src/ui/music/MusicWidgetAlbum.h \
    src/ui/music/MusicWidgetArtist.h \
    src/ui/music/MusicWidget.h \
    src/ui/notifications/MacNotificationHandler.h \
    src/ui/notifications/NotificationBox.h \
    src/ui/notifications/Notificator.h \
    src/ui/renamer/RenamerDialog.h \
    src/ui/renamer/RenamerPlaceholders.h \
    src/ui/scrapers/concert/TmdbConcertConfigurationView.h \
    src/ui/scrapers/image/FanartTvConfigurationView.h \
    src/ui/scrapers/movie/AebnConfigurationView.h \
    src/ui/scrapers/movie/ImdbMovieConfigurationView.h \
    src/ui/scrapers/movie/TmdbMovieConfigurationView.h \
    src/ui/scrapers/music/UniversalMusicConfigurationView.h \
    src/ui/scrapers/ScraperManager.h \
    src/ui/scrapers/tv_show/FernsehserienDeConfigurationView.h \
    src/ui/scrapers/tv_show/ImdbTvConfigurationView.h \
    src/ui/scrapers/tv_show/TheTvDbConfigurationView.h \
    src/ui/scrapers/tv_show/TmdbTvConfigurationView.h \
    src/ui/scrapers/tv_show/TvMazeConfigurationView.h \
    src/ui/settings/ConcertScraperInfoWidget.h \
    src/ui/settings/ConcertSettingsWidget.h \
    src/ui/settings/CustomTvScraperSettingsWidget.h \
    src/ui/settings/ExportSettingsWidget.h \
    src/ui/settings/ExportTemplateWidget.h \
    src/ui/settings/GlobalSettingsWidget.h \
    src/ui/settings/ImageProviderInfoWidget.h \
    src/ui/settings/ImportSettingsWidget.h \
    src/ui/settings/KodiSettingsWidget.h \
    src/ui/settings/MovieScraperInfoWidget.h \
    src/ui/settings/MovieSettingsWidget.h \
    src/ui/settings/MusicScraperInfoWidget.h \
    src/ui/settings/MusicSettingsWidget.h \
    src/ui/settings/NetworkSettingsWidget.h \
    src/ui/settings/ScraperSettingsTable.h \
    src/ui/settings/ScraperSettingsWidget.h \
    src/ui/settings/SettingsWindow.h \
    src/ui/settings/TvScraperInfoWidget.h \
    src/ui/settings/TvShowSettingsWidget.h \
    src/ui/small_widgets/ActorsWidget.h \
    src/ui/small_widgets/AlphabeticalList.h \
    src/ui/small_widgets/Badge.h \
    src/ui/small_widgets/ClosableImage.h \
    src/ui/small_widgets/ComboDelegate.h \
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
    src/ui/small_widgets/PlaceholderLineEdit.h \
    src/ui/small_widgets/RatingSourceDelegate.h \
    src/ui/small_widgets/RatingsWidget.h \
    src/ui/small_widgets/ScrapePreview.h \
    src/ui/small_widgets/SearchOverlay.h \
    src/ui/small_widgets/SlidingStackedWidget.h \
    src/ui/small_widgets/SpinBoxDelegate.h \
    src/ui/small_widgets/StereoModeComboBox.h \
    src/ui/small_widgets/TagCloud.h \
    src/ui/small_widgets/TvShowTreeView.h \
    src/ui/small_widgets/WebImageLabel.h \
    src/ui/support/SupportDialog.h \
    src/ui/trailer/TrailerDialog.h \
    src/ui/tv_show/search_dialog/TvShowPreviewAdapter.h \
    src/ui/tv_show/TvShowCommonWidgets.h \
    src/ui/tv_show/TvShowFilesWidget.h \
    src/ui/tv_show/TvShowMultiScrapeDialog.h \
    src/ui/tv_show/TvShowSearch.h \
    src/ui/tv_show/TvShowSearchWidget.h \
    src/ui/tv_show/TvShowWidgetEpisode.h \
    src/ui/tv_show/TvShowWidget.h \
    src/ui/tv_show/TvShowWidgetSeason.h \
    src/ui/tv_show/TvShowWidgetTvShow.h \
    src/ui/tv_show/TvTunesDialog.h \
    src/ui/UiUtils.h \
    src/utils/Containers.h \
    src/utils/Math.h \
    src/utils/Meta.h \
    src/utils/Random.h \
    src/utils/Time.h \
    src/workers/Job.h

FORMS += \
    src/ui/concerts/ConcertFilesWidget.ui \
    src/ui/concerts/ConcertInfoWidget.ui \
    src/ui/concerts/ConcertSearch.ui \
    src/ui/concerts/ConcertSearchWidget.ui \
    src/ui/concerts/ConcertStreamDetailsWidget.ui \
    src/ui/concerts/ConcertWidget.ui \
    src/ui/export/csv_export/CsvExportDialog.ui \
    src/ui/export/ExportDialog.ui \
    src/ui/image/ImageDialog.ui \
    src/ui/image/ImagePreviewDialog.ui \
    src/ui/import/DownloadsWidget.ui \
    src/ui/import/ImportActions.ui \
    src/ui/import/ImportDialog.ui \
    src/ui/import/MakeMkvDialog.ui \
    src/ui/import/UnpackButtons.ui \
    src/ui/main/AboutDialog.ui \
    src/ui/main/FileScannerDialog.ui \
    src/ui/main/MainWindow.ui \
    src/ui/main/Message.ui \
    src/ui/main/Navbar.ui \
    src/ui/media_center/KodiSync.ui \
    src/ui/movies/CertificationWidget.ui \
    src/ui/movie_sets/MovieListDialog.ui \
    src/ui/movie_sets/SetsWidget.ui \
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
    src/ui/music/MusicWidgetAlbum.ui \
    src/ui/music/MusicWidgetArtist.ui \
    src/ui/music/MusicWidget.ui \
    src/ui/notifications/NotificationBox.ui \
    src/ui/renamer/RenamerDialog.ui \
    src/ui/renamer/RenamerPlaceholders.ui \
    src/ui/settings/ConcertScraperInfoWidget.ui \
    src/ui/settings/ConcertSettingsWidget.ui \
    src/ui/settings/CustomTvScraperSettingsWidget.ui \
    src/ui/settings/ExportSettingsWidget.ui \
    src/ui/settings/ExportTemplateWidget.ui \
    src/ui/settings/GlobalSettingsWidget.ui \
    src/ui/settings/ImageProviderInfoWidget.ui \
    src/ui/settings/ImportSettingsWidget.ui \
    src/ui/settings/KodiSettingsWidget.ui \
    src/ui/settings/MovieScraperInfoWidget.ui \
    src/ui/settings/MovieSettingsWidget.ui \
    src/ui/settings/MusicScraperInfoWidget.ui \
    src/ui/settings/MusicSettingsWidget.ui \
    src/ui/settings/NetworkSettingsWidget.ui \
    src/ui/settings/ScraperSettingsTable.ui \
    src/ui/settings/ScraperSettingsWidget.ui \
    src/ui/settings/SettingsWindow.ui \
    src/ui/settings/TvScraperInfoWidget.ui \
    src/ui/settings/TvShowSettingsWidget.ui \
    src/ui/small_widgets/ActorsWidget.ui \
    src/ui/small_widgets/FilterWidget.ui \
    src/ui/small_widgets/ImageLabel.ui \
    src/ui/small_widgets/LoadingStreamDetails.ui \
    src/ui/small_widgets/MediaFlags.ui \
    src/ui/small_widgets/RatingsWidget.ui \
    src/ui/small_widgets/ScrapePreview.ui \
    src/ui/small_widgets/TagCloud.ui \
    src/ui/small_widgets/WebImageLabel.ui \
    src/ui/support/SupportDialog.ui \
    src/ui/trailer/TrailerDialog.ui \
    src/ui/tv_show/TvShowFilesWidget.ui \
    src/ui/tv_show/TvShowMultiScrapeDialog.ui \
    src/ui/tv_show/TvShowSearch.ui \
    src/ui/tv_show/TvShowSearchWidget.ui \
    src/ui/tv_show/TvShowWidgetEpisode.ui \
    src/ui/tv_show/TvShowWidgetSeason.ui \
    src/ui/tv_show/TvShowWidgetTvShow.ui \
    src/ui/tv_show/TvShowWidget.ui \
    src/ui/tv_show/TvTunesDialog.ui

macx {
    HEADERS += src/ui/notifications/MacNotificationHandler.h \
        src/ui/MacUiUtilities.h
    OBJECTIVE_SOURCES += src/ui/notifications/MacNotificationHandler.mm \
        src/ui/MacUiUtilities.mm
}

RESOURCES += data/MediaElch.qrc ui.qrc

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
    DEFINES += MEDIAELCH_USE_ASAN_STACKTRACE

} else {
    message(Normal build)
}
