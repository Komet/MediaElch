#-------------------------------------------------
#
# Project created by QtCreator 2012-02-16T11:13:07
#
#-------------------------------------------------

QT       += core gui network script xml sql

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

QMAKE_CFLAGS += -gdwarf-2
QMAKE_CXXFLAGS += -gdwarf-2

SOURCES += main.cpp\
        main/MainWindow.cpp \
    data/Movie.cpp \
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
    smallWidgets/FilterWidget.cpp \
    smallWidgets/MyTableWidget.cpp \
    main/MessageBox.cpp \
    main/Message.cpp \
    data/TvShow.cpp \
    data/TvShowFileSearcher.cpp \
    data/TvShowModel.cpp \
    tvShows/TvShowFilesWidget.cpp \
    data/TvShowProxyModel.cpp \
    data/TvShowModelItem.cpp \
    data/TvShowEpisode.cpp \
    data/TvShowDelegate.cpp \
    tvShows/TvShowWidget.cpp \
    tvShows/TvShowWidgetEpisode.cpp \
    tvShows/TvShowWidgetTvShow.cpp \
    scrapers/TheTvDb.cpp \
    tvShows/TvShowSearch.cpp \
    mediaCenterPlugins/XbmcSql.cpp \
    globals/DownloadManagerElement.cpp \
    smallWidgets/ImageLabel.cpp \
    globals/ImagePreviewDialog.cpp \
    sets/SetsWidget.cpp \
    sets/MovieListDialog.cpp \
    globals/Helper.cpp \
    smallWidgets/MyTreeView.cpp \
    data/MovieDelegate.cpp \
    globals/ImageDialog.cpp \
    settings/DataFile.cpp \
    data/ConcertFileSearcher.cpp \
    data/ConcertModel.cpp \
    data/ConcertDelegate.cpp \
    data/ConcertProxyModel.cpp \
    data/Concert.cpp \
    concerts/ConcertWidget.cpp \
    concerts/ConcertSearch.cpp \
    concerts/ConcertFilesWidget.cpp \
    scrapers/TMDbConcerts.cpp \
    cli/CLI.cpp \
    settings/SettingsWidget.cpp \
    settings/Settings.cpp \
    movies/GenreWidget.cpp \
    movies/CertificationWidget.cpp \
    support/SupportDialog.cpp \
    main/FileScannerDialog.cpp \
    globals/Filter.cpp

HEADERS  += main/MainWindow.h \
    data/Movie.h \
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
    main/MessageBox.h \
    main/Message.h \
    data/TvShow.h \
    data/TvShowFileSearcher.h \
    data/TvShowModel.h \
    tvShows/TvShowFilesWidget.h \
    data/TvShowProxyModel.h \
    data/TvShowModelItem.h \
    data/TvShowEpisode.h \
    data/TvShowDelegate.h \
    tvShows/TvShowWidget.h \
    tvShows/TvShowWidgetEpisode.h \
    tvShows/TvShowWidgetTvShow.h \
    scrapers/TheTvDb.h \
    data/TvScraperInterface.h \
    tvShows/TvShowSearch.h \
    mediaCenterPlugins/XbmcSql.h \
    globals/DownloadManagerElement.h \
    smallWidgets/ImageLabel.h \
    globals/ImagePreviewDialog.h \
    sets/SetsWidget.h \
    sets/MovieListDialog.h \
    globals/Helper.h \
    smallWidgets/MyTreeView.h \
    data/MovieDelegate.h \
    globals/ImageDialog.h \
    settings/DataFile.h \
    data/ConcertFileSearcher.h \
    data/ConcertModel.h \
    data/ConcertDelegate.h \
    data/ConcertProxyModel.h \
    data/Concert.h \
    data/ConcertScraperInterface.h \
    concerts/ConcertWidget.h \
    concerts/ConcertSearch.h \
    concerts/ConcertFilesWidget.h \
    scrapers/TMDbConcerts.h \
    cli/CLI.h \
    settings/SettingsWidget.h \
    settings/Settings.h \
    movies/GenreWidget.h \
    movies/CertificationWidget.h \
    support/SupportDialog.h \
    main/FileScannerDialog.h \
    globals/Filter.h

FORMS    += main/MainWindow.ui \
    movies/MovieSearch.ui \
    movies/MovieWidget.ui \
    movies/FilesWidget.ui \
    main/AboutDialog.ui \
    smallWidgets/FilterWidget.ui \
    main/MessageBox.ui \
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
    settings/SettingsWidget.ui \
    movies/GenreWidget.ui \
    movies/CertificationWidget.ui \
    support/SupportDialog.ui \
    main/FileScannerDialog.ui

RESOURCES += \
    MediaElch.qrc

TRANSLATIONS += \
    i18n/MediaElch_de.ts
