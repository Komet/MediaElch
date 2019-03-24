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
    src/tv_shows/TvShow.cpp \
    src/tv_shows/TvShowEpisode.cpp \
    src/tv_shows/TvShowFileSearcher.cpp \
    src/ui/downloads/DownloadsWidget.cpp \
    src/downloads/Extractor.cpp \
    src/downloads/FileWorker.cpp \
    src/ui/downloads/ImportActions.cpp \
    src/ui/downloads/ImportDialog.cpp \
    src/downloads/MakeMkvCon.cpp \
    src/ui/downloads/MakeMkvDialog.cpp \
    src/downloads/MyFile.cpp \
    src/ui/downloads/UnpackButtons.cpp \
    src/ui/export/ExportDialog.cpp \
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
    src/ui/image/ImageWidget.cpp \
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
    src/media_centers/KodiXml.cpp \
    src/media_centers/kodi/ArtistXmlReader.cpp \
    src/media_centers/kodi/ArtistXmlWriter.cpp \
    src/media_centers/kodi/ConcertXmlWriter.cpp \
    src/media_centers/kodi/ConcertXmlReader.cpp \
    src/media_centers/kodi/EpisodeXmlReader.cpp \
    src/media_centers/kodi/MovieXmlWriter.cpp \
    src/media_centers/kodi/MovieXmlReader.cpp \
    src/media_centers/kodi/TvShowXmlWriter.cpp \
    src/media_centers/kodi/TvShowXmlReader.cpp \
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
    src/ui/movie_sets/MovieListDialog.cpp \
    src/ui/movie_sets/SetsWidget.cpp \
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
    src/ui/support/SupportDialog.cpp \
    src/scrapers/trailer/HdTrailers.cpp \
    src/ui/tv_show/TvShowMultiScrapeDialog.cpp \
    src/ui/tv_show/TvShowSearch.cpp \
    src/ui/tv_show/TvShowSearchEpisode.cpp \
    src/tv_shows/TvShowUpdater.cpp \
    src/ui/tv_show/TvShowWidget.cpp \
    src/ui/tv_show/TvShowWidgetEpisode.cpp \
    src/ui/tv_show/TvShowWidgetSeason.cpp \
    src/ui/tv_show/TvShowWidgetTvShow.cpp \
    src/ui/tv_show/TvTunesDialog.cpp \
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
    src/tv_shows/EpisodeNumber.cpp \
    src/tv_shows/SeasonNumber.cpp \
    src/data/Certification.cpp \
    src/movies/MovieCrew.cpp

macx {
    OBJECTIVE_SOURCES += src/notifications/MacNotificationHandler.mm
}

HEADERS  += Version.h \
    src/concerts/ConcertController.h \
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
    src/media_centers/MediaCenterInterface.h \
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
    src/tv_shows/TvShow.h \
    src/tv_shows/TvShowEpisode.h \
    src/tv_shows/TvShowFileSearcher.h \
    src/ui/downloads/DownloadsWidget.h \
    src/downloads/Extractor.h \
    src/downloads/FileWorker.h \
    src/ui/downloads/ImportActions.h \
    src/ui/downloads/ImportDialog.h \
    src/downloads/MakeMkvCon.h \
    src/ui/downloads/MakeMkvDialog.h \
    src/downloads/MyFile.h \
    src/ui/downloads/UnpackButtons.h \
    src/ui/export/ExportDialog.h \
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
    src/ui/image/ImageWidget.h \
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
    src/media_centers/KodiXml.h \
    src/media_centers/kodi/ArtistXmlReader.h \
    src/media_centers/kodi/ArtistXmlWriter.h \
    src/media_centers/kodi/ConcertXmlWriter.h \
    src/media_centers/kodi/ConcertXmlReader.h \
    src/media_centers/kodi/EpisodeXmlReader.h \
    src/media_centers/kodi/MovieXmlWriter.h \
    src/media_centers/kodi/MovieXmlReader.h \
    src/media_centers/kodi/TvShowXmlWriter.h \
    src/media_centers/kodi/TvShowXmlReader.h \
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
    src/ui/movie_sets/MovieListDialog.h \
    src/ui/movie_sets/SetsWidget.h \
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
    src/ui/support/SupportDialog.h \
    src/scrapers/trailer/HdTrailers.h \
    src/scrapers/trailer/TrailerProvider.h \
    src/ui/tv_show/TvShowMultiScrapeDialog.h \
    src/ui/tv_show/TvShowSearch.h \
    src/ui/tv_show/TvShowSearchEpisode.h \
    src/tv_shows/TvShowUpdater.h \
    src/ui/tv_show/TvShowWidget.h \
    src/ui/tv_show/TvShowWidgetEpisode.h \
    src/ui/tv_show/TvShowWidgetSeason.h \
    src/ui/tv_show/TvShowWidgetTvShow.h \
    src/ui/tv_show/TvTunesDialog.h \
    src/tv_shows/TvShowModel.h \
    src/tv_shows/TvShowProxyModel.h \
    src/tv_shows/model/TvShowModelItem.h \
    src/tv_shows/model/TvShowBaseModelItem.h \
    src/tv_shows/model/TvShowRootModelItem.h \
    src/tv_shows/model/EpisodeModelItem.h \
    src/tv_shows/model/SeasonModelItem.h \
    src/ui/media_centers/KodiSync.h \
    src/data/ImdbId.h \
    src/data/TmdbId.h \
    src/tv_shows/TvDbId.h \
    src/tv_shows/EpisodeNumber.h \
    src/tv_shows/SeasonNumber.h \
    src/data/Certification.h \
    src/movies/MovieCrew.h

FORMS    += src/main/MainWindow.ui \
    src/ui/concerts/ConcertFilesWidget.ui \
    src/ui/concerts/ConcertSearch.ui \
    src/ui/concerts/ConcertSearchWidget.ui \
    src/ui/concerts/ConcertStreamDetailsWidget.ui \
    src/ui/concerts/ConcertWidget.ui \
    src/ui/concerts/ConcertInfoWidget.ui \
    src/ui/downloads/DownloadsWidget.ui \
    src/ui/downloads/ImportActions.ui \
    src/ui/downloads/ImportDialog.ui \
    src/ui/downloads/MakeMkvDialog.ui \
    src/ui/downloads/UnpackButtons.ui \
    src/ui/export/ExportDialog.ui \
    src/globals/ImageDialog.ui \
    src/globals/ImagePreviewDialog.ui \
    src/globals/TrailerDialog.ui \
    src/ui/image/ImageWidget.ui \
    src/main/AboutDialog.ui \
    src/main/FileScannerDialog.ui \
    src/main/Message.ui \
    src/main/Navbar.ui \
    src/ui/movies/CertificationWidget.ui \
    src/ui/movies/GenreWidget.ui \
    src/ui/movies/MovieDuplicateItem.ui \
    src/ui/movies/MovieDuplicates.ui \
    src/ui/movies/MovieFilesWidget.ui \
    src/ui/movies/MovieMultiScrapeDialog.ui \
    src/ui/movies/MovieSearch.ui \
    src/ui/movies/MovieSearchWidget.ui \
    src/ui/movies/MovieWidget.ui \
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
    src/ui/movie_sets/MovieListDialog.ui \
    src/ui/movie_sets/SetsWidget.ui \
    src/settings/ExportTemplateWidget.ui \
    src/settings/SettingsWindow.ui \
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
    src/ui/tv_show/TvShowSearchEpisode.ui \
    src/ui/tv_show/TvShowWidget.ui \
    src/ui/tv_show/TvShowWidgetEpisode.ui \
    src/ui/tv_show/TvShowWidgetSeason.ui \
    src/ui/tv_show/TvShowWidgetTvShow.ui \
    src/ui/tv_show/TvTunesDialog.ui \
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
