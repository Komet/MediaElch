#-------------------------------------------------
#
# Project created by QtCreator 2012-02-16T11:13:07
#
#-------------------------------------------------

include(quazip/quazip/quazip.pri)

QT       += core gui network script xml sql widgets multimedia multimediawidgets concurrent qml quick quickwidgets opengl

LIBS += -lzen -lz -lmediainfo

contains(DEFINES, PLUGINS){
    LIBS += -lqca
}

unix:LIBS += -lcurl
macx:LIBS += -framework Foundation
unix:!macx {
    LIBS += -ldl
}
win32 {
    DEFINES+=_UNICODE
}

TARGET = MediaElch
TEMPLATE = app

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

SOURCES += main.cpp\
        main/MainWindow.cpp \
    movies/Movie.cpp \
    data/MovieFileSearcher.cpp \
    mediaCenterPlugins/XbmcXml.cpp \
    scrapers/TMDb.cpp \
    globals/Manager.cpp \
    movies/MovieSearch.cpp \
    smallWidgets/MyLineEdit.cpp \
    movies/MovieWidget.cpp \
    smallWidgets/MyLabel.cpp \
    globals/DownloadManager.cpp \
    movies/FilesWidget.cpp \
    data/MovieModel.cpp \
    data/MovieProxyModel.cpp \
    main/AboutDialog.cpp \
    scrapers/VideoBuster.cpp \
    scrapers/OFDb.cpp \
    scrapers/Cinefacts.cpp \
    smallWidgets/MyTableWidget.cpp \
    notifications/NotificationBox.cpp \
    main/Message.cpp \
    data/TvShow.cpp \
    data/TvShowFileSearcher.cpp \
    data/TvShowModel.cpp \
    tvShows/TvShowFilesWidget.cpp \
    data/TvShowProxyModel.cpp \
    data/TvShowModelItem.cpp \
    data/TvShowEpisode.cpp \
    tvShows/TvShowWidget.cpp \
    tvShows/TvShowWidgetEpisode.cpp \
    tvShows/TvShowWidgetTvShow.cpp \
    scrapers/TheTvDb.cpp \
    tvShows/TvShowSearch.cpp \
    globals/DownloadManagerElement.cpp \
    smallWidgets/ImageLabel.cpp \
    globals/ImagePreviewDialog.cpp \
    sets/SetsWidget.cpp \
    sets/MovieListDialog.cpp \
    globals/Helper.cpp \
    smallWidgets/MyTreeView.cpp \
    globals/ImageDialog.cpp \
    settings/DataFile.cpp \
    data/ConcertFileSearcher.cpp \
    data/ConcertModel.cpp \
    data/ConcertProxyModel.cpp \
    data/Concert.cpp \
    concerts/ConcertWidget.cpp \
    concerts/ConcertSearch.cpp \
    concerts/ConcertFilesWidget.cpp \
    scrapers/TMDbConcerts.cpp \
    settings/Settings.cpp \
    movies/GenreWidget.cpp \
    movies/CertificationWidget.cpp \
    support/SupportDialog.cpp \
    main/FileScannerDialog.cpp \
    globals/Filter.cpp \
    data/MovieFilesOrganizer.cpp \
    globals/NameFormatter.cpp \
    imageProviders/FanartTv.cpp \
    imageProviders/FanartTvMusicArtists.cpp \
    imageProviders/TMDbImages.cpp \
    imageProviders/TheTvDbImages.cpp \
    globals/ComboDelegate.cpp \
    data/StreamDetails.cpp \
    smallWidgets/MediaFlags.cpp \
    data/Database.cpp \
    smallWidgets/LoadingStreamDetails.cpp \
    trailerProviders/MovieMaze.cpp \
    globals/TrailerDialog.cpp \
    smallWidgets/SlidingStackedWidget.cpp \
    scrapers/IMDB.cpp \
    xbmc/XbmcSync.cpp \
    smallWidgets/MyCheckBox.cpp \
    movies/MovieController.cpp \
    movies/MovieMultiScrapeDialog.cpp \
    smallWidgets/Badge.cpp \
    trailerProviders/HdTrailers.cpp \
    smallWidgets/TagCloud.cpp \
    smallWidgets/MyWidget.cpp \
    data/Storage.cpp \
    tvShows/TvShowWidgetSeason.cpp \
    smallWidgets/ImageGallery.cpp \
    smallWidgets/ClosableImage.cpp \
    renamer/Renamer.cpp \
    smallWidgets/MySpinBox.cpp \
    settings/AdvancedSettings.cpp \
    smallWidgets/AlphabeticalList.cpp \
    smallWidgets/MyTableView.cpp \
    data/ImageCache.cpp \
    scrapers/TvTunes.cpp \
    tvShows/TvTunesDialog.cpp \
    settings/SettingsWindow.cpp \
    export/ExportTemplateLoader.cpp \
    export/ExportTemplate.cpp \
    settings/ExportTemplateWidget.cpp \
    export/ExportDialog.cpp \
    smallWidgets/MessageLabel.cpp \
    smallWidgets/SearchOverlay.cpp \
    scrapers/CustomMovieScraper.cpp \
    scrapers/MediaPassion.cpp \
    imageProviders/MediaPassionImages.cpp \
    downloads/DownloadsWidget.cpp \
    smallWidgets/MyTableWidgetItem.cpp \
    downloads/UnpackButtons.cpp \
    downloads/Extractor.cpp \
    downloads/ImportActions.cpp \
    movies/MovieSearchWidget.cpp \
    downloads/ImportDialog.cpp \
    downloads/FileWorker.cpp \
    downloads/MyFile.cpp \
    concerts/ConcertSearchWidget.cpp \
    concerts/ConcertController.cpp \
    smallWidgets/MySplitter.cpp \
    smallWidgets/MySplitterHandle.cpp \
    tvShows/TvShowSearchEpisode.cpp \
    notifications/Notificator.cpp \
    main/Update.cpp \
    tvShows/TvShowUpdater.cpp \
    scrapers/AEBN.cpp \
    scrapers/HotMovies.cpp \
    scrapers/AdultDvdEmpire.cpp \
    main/Navbar.cpp \
    smallWidgets/FilterWidget.cpp \
    downloads/MakeMkvDialog.cpp \
    downloads/MakeMkvCon.cpp \
    plugins/PluginManager.cpp \
    plugins/PluginsWidget.cpp \
    plugins/PluginManagerDialog.cpp \
    music/Artist.cpp \
    music/Album.cpp \
    music/MusicModel.cpp \
    music/MusicModelItem.cpp \
    music/MusicFileSearcher.cpp \
    music/MusicWidget.cpp \
    music/MusicFilesWidget.cpp \
    music/ArtistController.cpp \
    music/AlbumController.cpp \
    music/MusicWidgetArtist.cpp \
    music/MusicWidgetAlbum.cpp \
    imageProviders/FanartTvMusic.cpp \
    music/MusicSearch.cpp \
    music/MusicSearchWidget.cpp \
    main/MyIconFont.cpp \
    music/MusicProxyModel.cpp \
    smallWidgets/MusicTreeView.cpp \
    scrapers/UniversalMusicScraper.cpp \
    music/MusicMultiScrapeDialog.cpp \
    renamer/RenamerPlaceholders.cpp \
    image/Image.cpp \
    image/ImageModel.cpp \
    image/ImageProxyModel.cpp \
    qml/AlbumImageProvider.cpp \
    image/ImageWidget.cpp \
    imageProviders/Coverlib.cpp \
    globals/NetworkReplyWatcher.cpp \
    smallWidgets/TvShowTreeView.cpp \
    tvShows/TvShowMultiScrapeDialog.cpp \
    data/Subtitle.cpp \
    image/ImageCapture.cpp

macx {
    OBJECTIVE_SOURCES += notifications/MacNotificationHandler.mm
}

HEADERS  += main/MainWindow.h \
    movies/Movie.h \
    globals/Globals.h \
    data/MediaCenterInterface.h \
    data/MovieFileSearcher.h \
    mediaCenterPlugins/XbmcXml.h \
    scrapers/TMDb.h \
    data/ScraperInterface.h \
    globals/Manager.h \
    movies/MovieSearch.h \
    smallWidgets/MyLineEdit.h \
    movies/MovieWidget.h \
    smallWidgets/MyLabel.h \
    globals/DownloadManager.h \
    movies/FilesWidget.h \
    data/MovieModel.h \
    data/MovieProxyModel.h \
    main/AboutDialog.h \
    scrapers/VideoBuster.h \
    scrapers/OFDb.h \
    scrapers/Cinefacts.h \
    smallWidgets/FilterWidget.h \
    smallWidgets/MyTableWidget.h \
    notifications/NotificationBox.h \
    main/Message.h \
    data/TvShow.h \
    data/TvShowFileSearcher.h \
    data/TvShowModel.h \
    tvShows/TvShowFilesWidget.h \
    data/TvShowProxyModel.h \
    data/TvShowModelItem.h \
    data/TvShowEpisode.h \
    tvShows/TvShowWidget.h \
    tvShows/TvShowWidgetEpisode.h \
    tvShows/TvShowWidgetTvShow.h \
    scrapers/TheTvDb.h \
    data/TvScraperInterface.h \
    tvShows/TvShowSearch.h \
    globals/DownloadManagerElement.h \
    smallWidgets/ImageLabel.h \
    globals/ImagePreviewDialog.h \
    sets/SetsWidget.h \
    sets/MovieListDialog.h \
    globals/Helper.h \
    smallWidgets/MyTreeView.h \
    globals/ImageDialog.h \
    settings/DataFile.h \
    data/ConcertFileSearcher.h \
    data/ConcertModel.h \
    data/ConcertProxyModel.h \
    data/Concert.h \
    data/ConcertScraperInterface.h \
    concerts/ConcertWidget.h \
    concerts/ConcertSearch.h \
    concerts/ConcertFilesWidget.h \
    scrapers/TMDbConcerts.h \
    settings/Settings.h \
    movies/GenreWidget.h \
    movies/CertificationWidget.h \
    support/SupportDialog.h \
    main/FileScannerDialog.h \
    globals/Filter.h \
    data/MovieFilesOrganizer.h \
    globals/NameFormatter.h \
    data/ImageProviderInterface.h \
    imageProviders/FanartTv.h \
    imageProviders/FanartTvMusicArtists.h \
    imageProviders/TMDbImages.h \
    imageProviders/TheTvDbImages.h \
    globals/ComboDelegate.h \
    data/StreamDetails.h \
    smallWidgets/MediaFlags.h \
    data/Database.h \
    smallWidgets/LoadingStreamDetails.h \
    trailerProviders/TrailerProvider.h \
    trailerProviders/MovieMaze.h \
    globals/TrailerDialog.h \
    smallWidgets/SlidingStackedWidget.h \
    scrapers/IMDB.h \
    xbmc/XbmcSync.h \
    smallWidgets/MyCheckBox.h \
    movies/MovieController.h \
    movies/MovieMultiScrapeDialog.h \
    smallWidgets/Badge.h \
    trailerProviders/HdTrailers.h \
    smallWidgets/TagCloud.h \
    smallWidgets/MyWidget.h \
    data/Storage.h \
    tvShows/TvShowWidgetSeason.h \
    smallWidgets/ImageGallery.h \
    smallWidgets/ClosableImage.h \
    renamer/Renamer.h \
    smallWidgets/MySpinBox.h \
    settings/AdvancedSettings.h \
    smallWidgets/AlphabeticalList.h \
    smallWidgets/MyTableView.h \
    data/ImageCache.h \
    scrapers/TvTunes.h \
    tvShows/TvTunesDialog.h \
    globals/LocaleStringCompare.h \
    settings/SettingsWindow.h \
    export/ExportTemplateLoader.h \
    export/ExportTemplate.h \
    settings/ExportTemplateWidget.h \
    export/ExportDialog.h \
    smallWidgets/MessageLabel.h \
    smallWidgets/SearchOverlay.h \
    scrapers/CustomMovieScraper.h \
    scrapers/MediaPassion.h \
    imageProviders/MediaPassionImages.h \
    downloads/DownloadsWidget.h \
    smallWidgets/MyTableWidgetItem.h \
    downloads/UnpackButtons.h \
    downloads/Extractor.h \
    downloads/ImportActions.h \
    movies/MovieSearchWidget.h \
    downloads/ImportDialog.h \
    downloads/FileWorker.h \
    downloads/MyFile.h \
    concerts/ConcertSearchWidget.h \
    concerts/ConcertController.h \
    smallWidgets/MySplitter.h \
    smallWidgets/MySplitterHandle.h \
    tvShows/TvShowSearchEpisode.h \
    notifications/Notificator.h \
    notifications/MacNotificationHandler.h \
    main/Update.h \
    tvShows/TvShowUpdater.h \
    scrapers/AEBN.h \
    scrapers/HotMovies.h \
    scrapers/AdultDvdEmpire.h \
    main/Navbar.h \
    downloads/MakeMkvDialog.h \
    downloads/MakeMkvCon.h \
    plugins/PluginInterface.h \
    plugins/PluginManager.h \
    plugins/PluginsWidget.h \
    plugins/PluginManagerDialog.h \
    music/Artist.h \
    music/Album.h \
    music/MusicModel.h \
    music/MusicModelItem.h \
    music/MusicFileSearcher.h \
    music/MusicWidget.h \
    music/MusicFilesWidget.h \
    music/ArtistController.h \
    music/AlbumController.h \
    music/MusicWidgetArtist.h \
    music/MusicWidgetAlbum.h \
    imageProviders/FanartTvMusic.h \
    music/MusicSearch.h \
    music/MusicSearchWidget.h \
    data/MusicScraperInterface.h \
    main/MyIconFont.h \
    music/MusicProxyModel.h \
    smallWidgets/MusicTreeView.h \
    scrapers/UniversalMusicScraper.h \
    music/MusicMultiScrapeDialog.h \
    renamer/RenamerPlaceholders.h \
    image/Image.h \
    image/ImageModel.h \
    image/ImageProxyModel.h \
    qml/AlbumImageProvider.h \
    image/ImageWidget.h \
    imageProviders/Coverlib.h \
    globals/NetworkReplyWatcher.h \
    smallWidgets/TvShowTreeView.h \
    tvShows/TvShowMultiScrapeDialog.h \
    data/Subtitle.h \
    image/ImageCapture.h

FORMS    += main/MainWindow.ui \
    movies/MovieSearch.ui \
    movies/MovieWidget.ui \
    movies/FilesWidget.ui \
    main/AboutDialog.ui \
    smallWidgets/FilterWidget.ui \
    notifications/NotificationBox.ui \
    main/Message.ui \
    tvShows/TvShowFilesWidget.ui \
    tvShows/TvShowWidget.ui \
    tvShows/TvShowWidgetEpisode.ui \
    tvShows/TvShowWidgetTvShow.ui \
    tvShows/TvShowSearch.ui \
    smallWidgets/ImageLabel.ui \
    globals/ImagePreviewDialog.ui \
    sets/SetsWidget.ui \
    sets/MovieListDialog.ui \
    globals/ImageDialog.ui \
    concerts/ConcertWidget.ui \
    concerts/ConcertSearch.ui \
    concerts/ConcertFilesWidget.ui \
    movies/GenreWidget.ui \
    movies/CertificationWidget.ui \
    support/SupportDialog.ui \
    main/FileScannerDialog.ui \
    smallWidgets/MediaFlags.ui \
    smallWidgets/LoadingStreamDetails.ui \
    globals/TrailerDialog.ui \
    xbmc/XbmcSync.ui \
    movies/MovieMultiScrapeDialog.ui \
    smallWidgets/TagCloud.ui \
    tvShows/TvShowWidgetSeason.ui \
    renamer/Renamer.ui \
    tvShows/TvTunesDialog.ui \
    settings/SettingsWindow.ui \
    settings/ExportTemplateWidget.ui \
    export/ExportDialog.ui \
    tvShows/ItemWidgetShow.ui \
    downloads/DownloadsWidget.ui \
    downloads/UnpackButtons.ui \
    downloads/ImportActions.ui \
    movies/MovieSearchWidget.ui \
    downloads/ImportDialog.ui \
    concerts/ConcertSearchWidget.ui \
    tvShows/TvShowSearchEpisode.ui \
    main/Navbar.ui \
    downloads/MakeMkvDialog.ui \
    plugins/PluginsWidget.ui \
    plugins/PluginManagerDialog.ui \
    music/MusicWidget.ui \
    music/MusicFilesWidget.ui \
    music/MusicWidgetArtist.ui \
    music/MusicWidgetAlbum.ui \
    music/MusicSearch.ui \
    music/MusicSearchWidget.ui \
    music/MusicMultiScrapeDialog.ui \
    renamer/RenamerPlaceholders.ui \
    image/ImageWidget.ui \
    tvShows/TvShowMultiScrapeDialog.ui

RESOURCES += \
    MediaElch.qrc \
    i18n.qrc \
    ui.qrc

TRANSLATIONS += \
    i18n/MediaElch_en.ts \
    i18n/MediaElch_de.ts \
    i18n/MediaElch_fr.ts \
    i18n/MediaElch_cs_CZ.ts \
    i18n/MediaElch_pt_BR.ts \
    i18n/MediaElch_ko.ts \
    i18n/MediaElch_no.ts \
    i18n/MediaElch_pl.ts \
    i18n/MediaElch_pt_PT.ts \
    i18n/MediaElch_nl_NL.ts \
    i18n/MediaElch_es_ES.ts \
    i18n/MediaElch_it.ts \
    i18n/MediaElch_fi.ts \
    i18n/MediaElch_zh_CN.ts \
    i18n/MediaElch_bg.ts \
    i18n/MediaElch_sv.ts \
    i18n/MediaElch_ru.ts \
    i18n/MediaElch_ja.ts \
    i18n/MediaElch_da.ts
