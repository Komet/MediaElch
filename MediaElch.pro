# Check Qt versions
lessThan(QT_MAJOR_VERSION, 5): error(Qt 4 is not supported!)
lessThan(QT_MINOR_VERSION, 5): error(Qt 5.5 or higher is required!)

include(quazip/quazip/quazip.pri)

TEMPLATE = app
TARGET = MediaElch
INCLUDEPATH += $$PWD/src

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
}

*-g++*|*-clang* {
    # Include all Qt modules using isystem so that warnings reagarding Qt files are ignored
    QMAKE_CXXFLAGS += -isystem "$$[QT_INSTALL_HEADERS]"
    for (inc, QT) {
        QMAKE_CXXFLAGS += -isystem \"$$[QT_INSTALL_HEADERS]/Qt$$system("echo $$inc | sed 's/.*/\u&/'")\"
    }
}

# Enable (all/most) warnings but ignore them for quazip files.
*-g++* {
    WARNINGS += -Wall -Wextra
    WARNINGS += -Wunknown-pragmas -Wundef -Wold-style-cast -Wuseless-cast
    WARNINGS += -Wdisabled-optimization -Wstrict-overflow=4
    WARNINGS += -Winit-self -Wpointer-arith
    WARNINGS += -Wlogical-op -Wunsafe-loop-optimizations
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
dotDesktop.files = desktop/MediaElch.desktop
INSTALLS += dotDesktop

icon.path = /usr/share/pixmaps
icon.files = desktop/MediaElch.png
INSTALLS += icon

ICON = MediaElch.icns
RC_FILE = MediaElch.rc
QMAKE_INFO_PLIST = MediaElch.plist

SOURCES += src/main.cpp \
    src/concerts/ConcertController.cpp \
    src/concerts/ConcertFilesWidget.cpp \
    src/concerts/ConcertSearch.cpp \
    src/concerts/ConcertSearchWidget.cpp \
    src/concerts/ConcertWidget.cpp \
    src/data/Concert.cpp \
    src/data/ConcertFileSearcher.cpp \
    src/data/ConcertModel.cpp \
    src/data/ConcertProxyModel.cpp \
    src/data/Database.cpp \
    src/data/ImageCache.cpp \
    src/data/Movie.cpp \
    src/data/MovieFileSearcher.cpp \
    src/data/MovieFilesOrganizer.cpp \
    src/data/MovieImages.cpp \
    src/data/MovieModel.cpp \
    src/data/MovieProxyModel.cpp \
    src/data/Rating.cpp \
    src/data/Storage.cpp \
    src/data/StreamDetails.cpp \
    src/data/Subtitle.cpp \
    src/data/TvShow.cpp \
    src/data/TvShowEpisode.cpp \
    src/data/TvShowFileSearcher.cpp \
    src/data/TvShowModel.cpp \
    src/data/TvShowModelItem.cpp \
    src/data/TvShowProxyModel.cpp \
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
    src/imageProviders/FanartTv.cpp \
    src/imageProviders/FanartTvMusic.cpp \
    src/imageProviders/FanartTvMusicArtists.cpp \
    src/imageProviders/TheTvDbImages.cpp \
    src/imageProviders/TMDbImages.cpp \
    src/main/AboutDialog.cpp \
    src/main/FileScannerDialog.cpp \
    src/main/MainWindow.cpp \
    src/main/Message.cpp \
    src/main/MyIconFont.cpp \
    src/main/Navbar.cpp \
    src/main/Update.cpp \
    src/mediaCenterPlugins/XbmcXml.cpp \
    src/mediaCenterPlugins/kodi/ArtistXmlReader.cpp \
    src/mediaCenterPlugins/kodi/ArtistXmlWriter.cpp \
    src/mediaCenterPlugins/kodi/ConcertXmlWriter.cpp \
    src/mediaCenterPlugins/kodi/ConcertXmlReader.cpp \
    src/mediaCenterPlugins/kodi/EpisodeXmlReader.cpp \
    src/mediaCenterPlugins/kodi/MovieXmlWriter.cpp \
    src/mediaCenterPlugins/kodi/MovieXmlReader.cpp \
    src/mediaCenterPlugins/kodi/TvShowXmlWriter.cpp \
    src/mediaCenterPlugins/kodi/TvShowXmlReader.cpp \
    src/movies/CertificationWidget.cpp \
    src/movies/FilesWidget.cpp \
    src/movies/GenreWidget.cpp \
    src/movies/MovieController.cpp \
    src/movies/MovieDuplicateItem.cpp \
    src/movies/MovieDuplicates.cpp \
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
    src/scrapers/AdultDvdEmpire.cpp \
    src/scrapers/AEBN.cpp \
    src/scrapers/CustomMovieScraper.cpp \
    src/scrapers/HotMovies.cpp \
    src/scrapers/IMDB.cpp \
    src/scrapers/KinoDe.cpp \
    src/scrapers/OFDb.cpp \
    src/scrapers/TheTvDb.cpp \
    src/scrapers/TMDb.cpp \
    src/scrapers/TMDbConcerts.cpp \
    src/scrapers/TvTunes.cpp \
    src/scrapers/UniversalMusicScraper.cpp \
    src/scrapers/VideoBuster.cpp \
    src/sets/MovieListDialog.cpp \
    src/sets/SetsWidget.cpp \
    src/settings/AdvancedSettings.cpp \
    src/settings/DataFile.cpp \
    src/settings/ExportTemplateWidget.cpp \
    src/settings/Settings.cpp \
    src/settings/SettingsWindow.cpp \
    src/smallWidgets/AlphabeticalList.cpp \
    src/smallWidgets/Badge.cpp \
    src/smallWidgets/ClosableImage.cpp \
    src/smallWidgets/FilterWidget.cpp \
    src/smallWidgets/ImageGallery.cpp \
    src/smallWidgets/ImageLabel.cpp \
    src/smallWidgets/LoadingStreamDetails.cpp \
    src/smallWidgets/MediaFlags.cpp \
    src/smallWidgets/MessageLabel.cpp \
    src/smallWidgets/MusicTreeView.cpp \
    src/smallWidgets/MyCheckBox.cpp \
    src/smallWidgets/MyLabel.cpp \
    src/smallWidgets/MyLineEdit.cpp \
    src/smallWidgets/MySpinBox.cpp \
    src/smallWidgets/MySplitter.cpp \
    src/smallWidgets/MySplitterHandle.cpp \
    src/smallWidgets/MyTableView.cpp \
    src/smallWidgets/MyTableWidget.cpp \
    src/smallWidgets/MyTableWidgetItem.cpp \
    src/smallWidgets/MyTreeView.cpp \
    src/smallWidgets/MyWidget.cpp \
    src/smallWidgets/SearchOverlay.cpp \
    src/smallWidgets/SlidingStackedWidget.cpp \
    src/smallWidgets/TagCloud.cpp \
    src/smallWidgets/TvShowTreeView.cpp \
    src/support/SupportDialog.cpp \
    src/trailerProviders/HdTrailers.cpp \
    src/tvShows/TvShowFilesWidget.cpp \
    src/tvShows/TvShowMultiScrapeDialog.cpp \
    src/tvShows/TvShowSearch.cpp \
    src/tvShows/TvShowSearchEpisode.cpp \
    src/tvShows/TvShowUpdater.cpp \
    src/tvShows/TvShowWidget.cpp \
    src/tvShows/TvShowWidgetEpisode.cpp \
    src/tvShows/TvShowWidgetSeason.cpp \
    src/tvShows/TvShowWidgetTvShow.cpp \
    src/tvShows/TvTunesDialog.cpp \
    src/xbmc/XbmcSync.cpp \
    src/data/ImdbId.cpp \
    src/data/TmdbId.cpp \
    src/data/TvDbId.cpp \
    src/data/EpisodeNumber.cpp \
    src/data/SeasonNumber.cpp

macx {
    OBJECTIVE_SOURCES += src/notifications/MacNotificationHandler.mm
}

HEADERS  += Version.h \
    src/concerts/ConcertController.h \
    src/concerts/ConcertFilesWidget.h \
    src/concerts/ConcertSearch.h \
    src/concerts/ConcertSearchWidget.h \
    src/concerts/ConcertWidget.h \
    src/data/Concert.h \
    src/data/ConcertFileSearcher.h \
    src/data/ConcertModel.h \
    src/data/ConcertProxyModel.h \
    src/data/ConcertScraperInterface.h \
    src/data/Database.h \
    src/data/ImageCache.h \
    src/data/ImageProviderInterface.h \
    src/data/MediaCenterInterface.h \
    src/data/Movie.h \
    src/data/MovieFileSearcher.h \
    src/data/MovieFilesOrganizer.h \
    src/data/MovieImages.h \
    src/data/MovieModel.h \
    src/data/MovieProxyModel.h \
    src/data/MusicScraperInterface.h \
    src/data/Rating.h \
    src/data/ScraperInterface.h \
    src/data/Storage.h \
    src/data/StreamDetails.h \
    src/data/Subtitle.h \
    src/data/TvScraperInterface.h \
    src/data/TvShow.h \
    src/data/TvShowEpisode.h \
    src/data/TvShowFileSearcher.h \
    src/data/TvShowModel.h \
    src/data/TvShowModelItem.h \
    src/data/TvShowProxyModel.h \
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
    src/imageProviders/FanartTv.h \
    src/imageProviders/FanartTvMusic.h \
    src/imageProviders/FanartTvMusicArtists.h \
    src/imageProviders/TheTvDbImages.h \
    src/imageProviders/TMDbImages.h \
    src/main/AboutDialog.h \
    src/main/FileScannerDialog.h \
    src/main/MainWindow.h \
    src/main/Message.h \
    src/main/MyIconFont.h \
    src/main/Navbar.h \
    src/main/Update.h \
    src/mediaCenterPlugins/XbmcXml.h \
    src/mediaCenterPlugins/kodi/ArtistXmlReader.h \
    src/mediaCenterPlugins/kodi/ArtistXmlWriter.h \
    src/mediaCenterPlugins/kodi/ConcertXmlWriter.h \
    src/mediaCenterPlugins/kodi/ConcertXmlReader.h \
    src/mediaCenterPlugins/kodi/EpisodeXmlReader.h \
    src/mediaCenterPlugins/kodi/MovieXmlWriter.h \
    src/mediaCenterPlugins/kodi/MovieXmlReader.h \
    src/mediaCenterPlugins/kodi/TvShowXmlWriter.h \
    src/mediaCenterPlugins/kodi/TvShowXmlReader.h \
    src/movies/CertificationWidget.h \
    src/movies/FilesWidget.h \
    src/movies/GenreWidget.h \
    src/movies/MovieController.h \
    src/movies/MovieDuplicateItem.h \
    src/movies/MovieDuplicates.h \
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
    src/scrapers/AdultDvdEmpire.h \
    src/scrapers/AEBN.h \
    src/scrapers/CustomMovieScraper.h \
    src/scrapers/HotMovies.h \
    src/scrapers/IMDB.h \
    src/scrapers/KinoDe.h \
    src/scrapers/OFDb.h \
    src/scrapers/TheTvDb.h \
    src/scrapers/TMDb.h \
    src/scrapers/TMDbConcerts.h \
    src/scrapers/TvTunes.h \
    src/scrapers/UniversalMusicScraper.h \
    src/scrapers/VideoBuster.h \
    src/sets/MovieListDialog.h \
    src/sets/SetsWidget.h \
    src/settings/AdvancedSettings.h \
    src/settings/DataFile.h \
    src/settings/ExportTemplateWidget.h \
    src/settings/Settings.h \
    src/settings/SettingsWindow.h \
    src/smallWidgets/AlphabeticalList.h \
    src/smallWidgets/Badge.h \
    src/smallWidgets/ClosableImage.h \
    src/smallWidgets/FilterWidget.h \
    src/smallWidgets/ImageGallery.h \
    src/smallWidgets/ImageLabel.h \
    src/smallWidgets/LoadingStreamDetails.h \
    src/smallWidgets/MediaFlags.h \
    src/smallWidgets/MessageLabel.h \
    src/smallWidgets/MusicTreeView.h \
    src/smallWidgets/MyCheckBox.h \
    src/smallWidgets/MyLabel.h \
    src/smallWidgets/MyLineEdit.h \
    src/smallWidgets/MySpinBox.h \
    src/smallWidgets/MySplitter.h \
    src/smallWidgets/MySplitterHandle.h \
    src/smallWidgets/MyTableView.h \
    src/smallWidgets/MyTableWidget.h \
    src/smallWidgets/MyTableWidgetItem.h \
    src/smallWidgets/MyTreeView.h \
    src/smallWidgets/MyWidget.h \
    src/smallWidgets/SearchOverlay.h \
    src/smallWidgets/SlidingStackedWidget.h \
    src/smallWidgets/TagCloud.h \
    src/smallWidgets/TvShowTreeView.h \
    src/support/SupportDialog.h \
    src/trailerProviders/HdTrailers.h \
    src/trailerProviders/TrailerProvider.h \
    src/tvShows/TvShowFilesWidget.h \
    src/tvShows/TvShowMultiScrapeDialog.h \
    src/tvShows/TvShowSearch.h \
    src/tvShows/TvShowSearchEpisode.h \
    src/tvShows/TvShowUpdater.h \
    src/tvShows/TvShowWidget.h \
    src/tvShows/TvShowWidgetEpisode.h \
    src/tvShows/TvShowWidgetSeason.h \
    src/tvShows/TvShowWidgetTvShow.h \
    src/tvShows/TvTunesDialog.h \
    src/xbmc/XbmcSync.h \
    src/data/ImdbId.h \
    src/data/TmdbId.h \
    src/data/TvDbId.h \
    src/data/EpisodeNumber.h \
    src/data/SeasonNumber.h

FORMS    += src/main/MainWindow.ui \
    src/concerts/ConcertFilesWidget.ui \
    src/concerts/ConcertSearch.ui \
    src/concerts/ConcertSearchWidget.ui \
    src/concerts/ConcertWidget.ui \
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
    src/movies/FilesWidget.ui \
    src/movies/GenreWidget.ui \
    src/movies/MovieDuplicateItem.ui \
    src/movies/MovieDuplicates.ui \
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
    src/smallWidgets/FilterWidget.ui \
    src/smallWidgets/ImageLabel.ui \
    src/smallWidgets/LoadingStreamDetails.ui \
    src/smallWidgets/MediaFlags.ui \
    src/smallWidgets/TagCloud.ui \
    src/support/SupportDialog.ui \
    src/tvShows/ItemWidgetShow.ui \
    src/tvShows/TvShowFilesWidget.ui \
    src/tvShows/TvShowMultiScrapeDialog.ui \
    src/tvShows/TvShowSearch.ui \
    src/tvShows/TvShowSearchEpisode.ui \
    src/tvShows/TvShowWidget.ui \
    src/tvShows/TvShowWidgetEpisode.ui \
    src/tvShows/TvShowWidgetSeason.ui \
    src/tvShows/TvShowWidgetTvShow.ui \
    src/tvShows/TvTunesDialog.ui \
    src/xbmc/XbmcSync.ui

RESOURCES += \
    MediaElch.qrc \
    i18n.qrc \
    ui.qrc

TRANSLATIONS += \
    i18n/MediaElch_bg.ts \
    i18n/MediaElch_cs_CZ.ts \
    i18n/MediaElch_da.ts \
    i18n/MediaElch_de.ts \
    i18n/MediaElch_en.ts \
    i18n/MediaElch_es_ES.ts \
    i18n/MediaElch_fi.ts \
    i18n/MediaElch_fr.ts \
    i18n/MediaElch_it.ts \
    i18n/MediaElch_ja.ts \
    i18n/MediaElch_ko.ts \
    i18n/MediaElch_nl_NL.ts \
    i18n/MediaElch_no.ts \
    i18n/MediaElch_pl.ts \
    i18n/MediaElch_pt_BR.ts \
    i18n/MediaElch_pt_PT.ts \
    i18n/MediaElch_ru.ts \
    i18n/MediaElch_sv.ts \
    i18n/MediaElch_zh_CN.ts

test {
    message(Test build)

    QT += testlib
    TARGET = mediaelch-test

    HEADERS += test/test_helpers.h \
        test/helpers/matchers.h \
        test/helpers/debug_output.h

    SOURCES -= src/main.cpp
    SOURCES += test/main.cpp \
        test/helpers/matchers.cpp \
        test/scrapers/testAdultDvdEmpire.cpp \
        test/scrapers/testAEBN.cpp \
        test/scrapers/testHotMovies.cpp \
        test/scrapers/testIMDb.cpp \
        test/scrapers/testKinoDe.cpp \
        test/scrapers/testTMDb.cpp \
        test/scrapers/testVideoBuster.cpp

} else {
    sanitize {
        message(Sanitizer build)
        CONFIG += sanitizer sanitize_address

    } else {
        message(Normal build)
    }
}
