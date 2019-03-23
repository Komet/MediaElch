# Check Qt versions
lessThan(QT_MAJOR_VERSION, 5): error(Qt 4 is not supported!)
lessThan(QT_MINOR_VERSION, 5): error(Qt 5.5 or higher is required!)

DEFINES += QUAZIP_BUILD
DEFINES += QUAZIP_STATIC # Required by Quazip to export symbols
include(third_party/quazip/quazip/quazip.pri)

TEMPLATE = app
TARGET = MediaElch
INCLUDEPATH += $$PWD/src
INCLUDEPATH += $$PWD/third_party

QT += core gui network xml sql widgets multimedia multimediawidgets \
      concurrent qml quick quickwidgets opengl

CONFIG += warn_on c++14

LIBS += -lz

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
    src/concerts/ConcertFilesWidget.cpp \
    src/concerts/ConcertSearch.cpp \
    src/concerts/ConcertSearchWidget.cpp \
    src/concerts/ConcertStreamDetailsWidget.cpp \
    src/concerts/ConcertWidget.cpp \
    src/concerts/ConcertInfoWidget.cpp \
    src/data/Concert.cpp \
    src/data/ConcertFileSearcher.cpp \
    src/data/ConcertModel.cpp \
    src/data/ConcertProxyModel.cpp \
    src/data/Database.cpp \
    src/data/ImageCache.cpp \
    src/movies/Movie.cpp \
    src/movies/MovieFileSearcher.cpp \
    src/movies/MovieFilesOrganizer.cpp \
    src/movies/MovieImages.cpp \
    src/movies/MovieModel.cpp \
    src/movies/MovieProxyModel.cpp \
    src/data/Rating.cpp \
    src/scrapers/ScraperInterface.cpp \
    src/data/Storage.cpp \
    src/data/StreamDetails.cpp \
    src/data/Subtitle.cpp \
    src/data/TvShow.cpp \
    src/data/TvShowEpisode.cpp \
    src/data/TvShowFileSearcher.cpp \
    src/downloads/DownloadsWidget.cpp \
    src/downloads/Extractor.cpp \
    src/downloads/FileWorker.cpp \
    src/downloads/ImportActions.cpp \
    src/downloads/ImportDialog.cpp \
    src/downloads/MakeMkvCon.cpp \
    src/downloads/MakeMkvDialog.cpp \
    src/downloads/MyFile.cpp \
    src/downloads/UnpackButtons.cpp \
    src/export/ExportDialog.cpp \
    src/export/ExportTemplate.cpp \
    src/export/ExportTemplateLoader.cpp \
    src/globals/ComboDelegate.cpp \
    src/globals/DownloadManager.cpp \
    src/globals/DownloadManagerElement.cpp \
    src/globals/Filter.cpp \
    src/globals/Helper.cpp \
    src/globals/ImageDialog.cpp \
    src/globals/ImagePreviewDialog.cpp \
    src/globals/Manager.cpp \
    src/globals/NameFormatter.cpp \
    src/globals/NetworkReplyWatcher.cpp \
    src/globals/TrailerDialog.cpp \
    src/image/Image.cpp \
    src/image/ImageCapture.cpp \
    src/image/ImageModel.cpp \
    src/image/ImageProxyModel.cpp \
    src/image/ImageWidget.cpp \
    src/scrapers/image/FanartTv.cpp \
    src/scrapers/image/FanartTvMusic.cpp \
    src/scrapers/image/FanartTvMusicArtists.cpp \
    src/scrapers/image/TheTvDbImages.cpp \
    src/scrapers/image/TMDbImages.cpp \
    src/main/AboutDialog.cpp \
    src/main/FileScannerDialog.cpp \
    src/main/MainWindow.cpp \
    src/main/Message.cpp \
    src/main/MyIconFont.cpp \
    src/main/Navbar.cpp \
    src/main/Update.cpp \
    src/media_centers/XbmcXml.cpp \
    src/media_centers/kodi/ArtistXmlReader.cpp \
    src/media_centers/kodi/ArtistXmlWriter.cpp \
    src/media_centers/kodi/ConcertXmlWriter.cpp \
    src/media_centers/kodi/ConcertXmlReader.cpp \
    src/media_centers/kodi/EpisodeXmlReader.cpp \
    src/media_centers/kodi/MovieXmlWriter.cpp \
    src/media_centers/kodi/MovieXmlReader.cpp \
    src/media_centers/kodi/TvShowXmlWriter.cpp \
    src/media_centers/kodi/TvShowXmlReader.cpp \
    src/movies/CertificationWidget.cpp \
    src/movies/GenreWidget.cpp \
    src/movies/MovieController.cpp \
    src/movies/MovieDuplicateItem.cpp \
    src/movies/MovieDuplicates.cpp \
    src/movies/MovieFilesWidget.cpp \
    src/movies/MovieMultiScrapeDialog.cpp \
    src/movies/MovieSearch.cpp \
    src/movies/MovieSearchWidget.cpp \
    src/movies/MovieWidget.cpp \
    src/music/Album.cpp \
    src/music/AlbumController.cpp \
    src/music/Artist.cpp \
    src/music/ArtistController.cpp \
    src/music/MusicFileSearcher.cpp \
    src/music/MusicFilesWidget.cpp \
    src/music/MusicModel.cpp \
    src/music/MusicModelItem.cpp \
    src/music/MusicMultiScrapeDialog.cpp \
    src/music/MusicProxyModel.cpp \
    src/music/MusicSearch.cpp \
    src/music/MusicSearchWidget.cpp \
    src/music/MusicWidget.cpp \
    src/music/MusicWidgetAlbum.cpp \
    src/music/MusicWidgetArtist.cpp \
    src/notifications/NotificationBox.cpp \
    src/notifications/Notificator.cpp \
    src/qml/AlbumImageProvider.cpp \
    src/renamer/ConcertRenamer.cpp \
    src/renamer/EpisodeRenamer.cpp \
    src/renamer/MovieRenamer.cpp \
    src/renamer/Renamer.cpp \
    src/renamer/RenamerDialog.cpp \
    src/renamer/RenamerPlaceholders.cpp \
    src/scrapers/concert/TMDbConcerts.cpp \
    src/scrapers/movie/AdultDvdEmpire.cpp \
    src/scrapers/movie/AEBN.cpp \
    src/scrapers/movie/CustomMovieScraper.cpp \
    src/scrapers/movie/HotMovies.cpp \
    src/scrapers/movie/IMDB.cpp \
    src/scrapers/movie/OFDb.cpp \
    src/scrapers/movie/TMDb.cpp \
    src/scrapers/movie/VideoBuster.cpp \
    src/scrapers/music/TvTunes.cpp \
    src/scrapers/music/UniversalMusicScraper.cpp \
    src/scrapers/tv_show/TheTvDb.cpp \
    src/sets/MovieListDialog.cpp \
    src/sets/SetsWidget.cpp \
    src/settings/AdvancedSettings.cpp \
    src/settings/DataFile.cpp \
    src/settings/DirectorySettings.cpp \
    src/settings/ExportTemplateWidget.cpp \
    src/settings/ImportSettings.cpp \
    src/settings/KodiSettings.cpp \
    src/settings/NetworkSettings.cpp \
    src/settings/ScraperSettings.cpp \
    src/settings/Settings.cpp \
    src/settings/SettingsWindow.cpp \
    src/ui/small_widgets/AlphabeticalList.cpp \
    src/ui/small_widgets/Badge.cpp \
    src/ui/small_widgets/ClosableImage.cpp \
    src/ui/small_widgets/FilterWidget.cpp \
    src/ui/small_widgets/ImageGallery.cpp \
    src/ui/small_widgets/ImageLabel.cpp \
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
    src/support/SupportDialog.cpp \
    src/scrapers/trailer/HdTrailers.cpp \
    src/tvShows/TvShowMultiScrapeDialog.cpp \
    src/tvShows/TvShowSearch.cpp \
    src/tvShows/TvShowSearchEpisode.cpp \
    src/tvShows/TvShowUpdater.cpp \
    src/tvShows/TvShowWidget.cpp \
    src/tvShows/TvShowWidgetEpisode.cpp \
    src/tvShows/TvShowWidgetSeason.cpp \
    src/tvShows/TvShowWidgetTvShow.cpp \
    src/tvShows/TvTunesDialog.cpp \
    src/tvShows/TvShowModel.cpp \
    src/tvShows/TvShowProxyModel.cpp \
    src/tvShows/model/TvShowModelItem.cpp \
    src/tvShows/model/TvShowBaseModelItem.cpp \
    src/tvShows/model/TvShowRootModelItem.cpp \
    src/tvShows/model/EpisodeModelItem.cpp \
    src/tvShows/model/SeasonModelItem.cpp \
    src/ui/media_centers/KodiSync.cpp \
    src/data/ImdbId.cpp \
    src/data/TmdbId.cpp \
    src/data/TvDbId.cpp \
    src/data/EpisodeNumber.cpp \
    src/data/SeasonNumber.cpp \
    src/data/Certification.cpp \
    src/movies/MovieCrew.cpp

macx {
    OBJECTIVE_SOURCES += src/notifications/MacNotificationHandler.mm
}

HEADERS  += Version.h \
    src/concerts/ConcertController.h \
    src/concerts/ConcertFilesWidget.h \
    src/concerts/ConcertSearch.h \
    src/concerts/ConcertSearchWidget.h \
    src/concerts/ConcertWidget.h \
    src/concerts/ConcertInfoWidget.h \
    src/data/Concert.h \
    src/data/ConcertFileSearcher.h \
    src/data/ConcertModel.h \
    src/data/ConcertProxyModel.h \
    src/concerts/ConcertStreamDetailsWidget.h \
    src/data/Database.h \
    src/data/ImageCache.h \
    src/data/MediaCenterInterface.h \
    src/movies/Movie.h \
    src/movies/MovieFileSearcher.h \
    src/movies/MovieFilesOrganizer.h \
    src/movies/MovieImages.h \
    src/movies/MovieModel.h \
    src/movies/MovieProxyModel.h \
    src/scrapers/image/ImageProviderInterface.h \
    src/scrapers/concert/ConcertScraperInterface.h \
    src/scrapers/music/MusicScraperInterface.h \
    src/scrapers/movie/MovieScraperInterface.h \
    src/scrapers/tv_show/TvScraperInterface.h \
    src/scrapers/ScraperInterface.h \
    src/data/Rating.h \
    src/data/Storage.h \
    src/data/StreamDetails.h \
    src/data/Subtitle.h \
    src/data/TvShow.h \
    src/data/TvShowEpisode.h \
    src/data/TvShowFileSearcher.h \
    src/downloads/DownloadsWidget.h \
    src/downloads/Extractor.h \
    src/downloads/FileWorker.h \
    src/downloads/ImportActions.h \
    src/downloads/ImportDialog.h \
    src/downloads/MakeMkvCon.h \
    src/downloads/MakeMkvDialog.h \
    src/downloads/MyFile.h \
    src/downloads/UnpackButtons.h \
    src/export/ExportDialog.h \
    src/export/ExportTemplate.h \
    src/export/ExportTemplateLoader.h \
    src/globals/ComboDelegate.h \
    src/globals/DownloadManager.h \
    src/globals/DownloadManagerElement.h \
    src/globals/Filter.h \
    src/globals/Globals.h \
    src/globals/Helper.h \
    src/globals/ImageDialog.h \
    src/globals/ImagePreviewDialog.h \
    src/globals/LocaleStringCompare.h \
    src/globals/Manager.h \
    src/globals/NameFormatter.h \
    src/globals/NetworkReplyWatcher.h \
    src/globals/TrailerDialog.h \
    src/image/Image.h \
    src/image/ImageCapture.h \
    src/image/ImageModel.h \
    src/image/ImageProxyModel.h \
    src/image/ImageWidget.h \
    src/scrapers/image/FanartTv.h \
    src/scrapers/image/FanartTvMusic.h \
    src/scrapers/image/FanartTvMusicArtists.h \
    src/scrapers/image/TheTvDbImages.h \
    src/scrapers/image/TMDbImages.h \
    src/main/AboutDialog.h \
    src/main/FileScannerDialog.h \
    src/main/MainWindow.h \
    src/main/Message.h \
    src/main/MyIconFont.h \
    src/main/Navbar.h \
    src/main/Update.h \
    src/media_centers/XbmcXml.h \
    src/media_centers/kodi/ArtistXmlReader.h \
    src/media_centers/kodi/ArtistXmlWriter.h \
    src/media_centers/kodi/ConcertXmlWriter.h \
    src/media_centers/kodi/ConcertXmlReader.h \
    src/media_centers/kodi/EpisodeXmlReader.h \
    src/media_centers/kodi/MovieXmlWriter.h \
    src/media_centers/kodi/MovieXmlReader.h \
    src/media_centers/kodi/TvShowXmlWriter.h \
    src/media_centers/kodi/TvShowXmlReader.h \
    src/movies/CertificationWidget.h \
    src/movies/GenreWidget.h \
    src/movies/MovieController.h \
    src/movies/MovieDuplicateItem.h \
    src/movies/MovieDuplicates.h \
    src/movies/MovieFilesWidget.h \
    src/movies/MovieMultiScrapeDialog.h \
    src/movies/MovieSearch.h \
    src/movies/MovieSearchWidget.h \
    src/movies/MovieWidget.h \
    src/music/Album.h \
    src/music/AlbumController.h \
    src/music/Artist.h \
    src/music/ArtistController.h \
    src/music/MusicFileSearcher.h \
    src/music/MusicFilesWidget.h \
    src/music/MusicModel.h \
    src/music/MusicModelItem.h \
    src/music/MusicMultiScrapeDialog.h \
    src/music/MusicProxyModel.h \
    src/music/MusicSearch.h \
    src/music/MusicSearchWidget.h \
    src/music/MusicWidget.h \
    src/music/MusicWidgetAlbum.h \
    src/music/MusicWidgetArtist.h \
    src/notifications/MacNotificationHandler.h \
    src/notifications/NotificationBox.h \
    src/notifications/Notificator.h \
    src/qml/AlbumImageProvider.h \
    src/renamer/ConcertRenamer.h \
    src/renamer/EpisodeRenamer.h \
    src/renamer/MovieRenamer.h \
    src/renamer/Renamer.h \
    src/renamer/RenamerDialog.h \
    src/renamer/RenamerPlaceholders.h \
    src/scrapers/concert/TMDbConcerts.h \
    src/scrapers/movie/AdultDvdEmpire.h \
    src/scrapers/movie/AEBN.h \
    src/scrapers/movie/CustomMovieScraper.h \
    src/scrapers/movie/HotMovies.h \
    src/scrapers/movie/IMDB.h \
    src/scrapers/movie/OFDb.h \
    src/scrapers/movie/TMDb.h \
    src/scrapers/movie/VideoBuster.h \
    src/scrapers/music/TvTunes.h \
    src/scrapers/music/UniversalMusicScraper.h \
    src/scrapers/tv_show/TheTvDb.h \
    src/sets/MovieListDialog.h \
    src/sets/SetsWidget.h \
    src/settings/AdvancedSettings.h \
    src/settings/DataFile.h \
    src/settings/DirectorySettings.h \
    src/settings/ExportTemplateWidget.h \
    src/settings/ImportSettings.h \
    src/settings/KodiSettings.h \
    src/settings/NetworkSettings.h \
    src/settings/ScraperSettings.h \
    src/settings/Settings.h \
    src/settings/SettingsWindow.h \
    src/ui/small_widgets/AlphabeticalList.h \
    src/ui/small_widgets/Badge.h \
    src/ui/small_widgets/ClosableImage.h \
    src/ui/small_widgets/FilterWidget.h \
    src/ui/small_widgets/ImageGallery.h \
    src/ui/small_widgets/ImageLabel.h \
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
    src/support/SupportDialog.h \
    src/scrapers/trailer/HdTrailers.h \
    src/scrapers/trailer/TrailerProvider.h \
    src/tvShows/TvShowMultiScrapeDialog.h \
    src/tvShows/TvShowSearch.h \
    src/tvShows/TvShowSearchEpisode.h \
    src/tvShows/TvShowUpdater.h \
    src/tvShows/TvShowWidget.h \
    src/tvShows/TvShowWidgetEpisode.h \
    src/tvShows/TvShowWidgetSeason.h \
    src/tvShows/TvShowWidgetTvShow.h \
    src/tvShows/TvTunesDialog.h \
    src/tvShows/TvShowModel.h \
    src/tvShows/TvShowProxyModel.h \
    src/tvShows/model/TvShowModelItem.h \
    src/tvShows/model/TvShowBaseModelItem.h \
    src/tvShows/model/TvShowRootModelItem.h \
    src/tvShows/model/EpisodeModelItem.h \
    src/tvShows/model/SeasonModelItem.h \
    src/ui/media_centers/KodiSync.h \
    src/data/ImdbId.h \
    src/data/TmdbId.h \
    src/data/TvDbId.h \
    src/data/EpisodeNumber.h \
    src/data/SeasonNumber.h \
    src/data/Certification.h \
    src/movies/MovieCrew.h

FORMS    += src/main/MainWindow.ui \
    src/concerts/ConcertFilesWidget.ui \
    src/concerts/ConcertSearch.ui \
    src/concerts/ConcertSearchWidget.ui \
    src/concerts/ConcertStreamDetailsWidget.ui \
    src/concerts/ConcertWidget.ui \
    src/concerts/ConcertInfoWidget.ui \
    src/downloads/DownloadsWidget.ui \
    src/downloads/ImportActions.ui \
    src/downloads/ImportDialog.ui \
    src/downloads/MakeMkvDialog.ui \
    src/downloads/UnpackButtons.ui \
    src/export/ExportDialog.ui \
    src/globals/ImageDialog.ui \
    src/globals/ImagePreviewDialog.ui \
    src/globals/TrailerDialog.ui \
    src/image/ImageWidget.ui \
    src/main/AboutDialog.ui \
    src/main/FileScannerDialog.ui \
    src/main/Message.ui \
    src/main/Navbar.ui \
    src/movies/CertificationWidget.ui \
    src/movies/GenreWidget.ui \
    src/movies/MovieDuplicateItem.ui \
    src/movies/MovieDuplicates.ui \
    src/movies/MovieFilesWidget.ui \
    src/movies/MovieMultiScrapeDialog.ui \
    src/movies/MovieSearch.ui \
    src/movies/MovieSearchWidget.ui \
    src/movies/MovieWidget.ui \
    src/music/MusicFilesWidget.ui \
    src/music/MusicMultiScrapeDialog.ui \
    src/music/MusicSearch.ui \
    src/music/MusicSearchWidget.ui \
    src/music/MusicWidget.ui \
    src/music/MusicWidgetAlbum.ui \
    src/music/MusicWidgetArtist.ui \
    src/notifications/NotificationBox.ui \
    src/renamer/RenamerDialog.ui \
    src/renamer/RenamerPlaceholders.ui \
    src/sets/MovieListDialog.ui \
    src/sets/SetsWidget.ui \
    src/settings/ExportTemplateWidget.ui \
    src/settings/SettingsWindow.ui \
    src/ui/small_widgets/FilterWidget.ui \
    src/ui/small_widgets/ImageLabel.ui \
    src/ui/small_widgets/LoadingStreamDetails.ui \
    src/ui/small_widgets/MediaFlags.ui \
    src/ui/small_widgets/TagCloud.ui \
    src/ui/tv_show/TvShowFilesWidget.ui \
    src/support/SupportDialog.ui \
    src/tvShows/ItemWidgetShow.ui \
    src/tvShows/TvShowMultiScrapeDialog.ui \
    src/tvShows/TvShowSearch.ui \
    src/tvShows/TvShowSearchEpisode.ui \
    src/tvShows/TvShowWidget.ui \
    src/tvShows/TvShowWidgetEpisode.ui \
    src/tvShows/TvShowWidgetSeason.ui \
    src/tvShows/TvShowWidgetTvShow.ui \
    src/tvShows/TvTunesDialog.ui \
    src/ui/media_centers/KodiSync.ui

RESOURCES += \
    data/MediaElch.qrc \
    data/i18n.qrc \
    ui.qrc

TRANSLATIONS += \
    data/i18n/MediaElch_bg.ts \
    data/i18n/MediaElch_cs_CZ.ts \
    data/i18n/MediaElch_da.ts \
    data/i18n/MediaElch_de.ts \
    data/i18n/MediaElch_en.ts \
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

test {
    message(Test build)

    QT += testlib
    TARGET = mediaelch-test

    HEADERS += test/test_helpers.h \
        test/mocks/settings/MockScraperSettings.h \
        test/helpers/matchers.h \
        test/helpers/debug_output.h

    SOURCES -= src/main.cpp
    SOURCES += test/main.cpp \
        test/helpers/matchers.cpp \
        test/data/testCertification.cpp \
        test/data/testImdbId.cpp \
        test/mocks/settings/MockScraperSettings.cpp \
        test/scrapers/testAdultDvdEmpire.cpp \
        test/scrapers/testAEBN.cpp \
        test/scrapers/testHotMovies.cpp \
        test/scrapers/testIMDb.cpp \
        test/scrapers/testTMDb.cpp \
        test/scrapers/testVideoBuster.cpp \
        test/scrapers/testTMDbConcerts.cpp

} else {
    sanitize {
        message(Sanitizer build)
        CONFIG += sanitizer sanitize_address

    } else {
        message(Normal build)
    }
}
