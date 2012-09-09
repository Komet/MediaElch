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

SOURCES += main.cpp\
        MainWindow.cpp \
    data/Movie.cpp \
    data/MovieFileSearcher.cpp \
    mediaCenterPlugins/XbmcXml.cpp \
    scrapers/TMDb.cpp \
    Manager.cpp \
    MovieSearch.cpp \
    smallWidgets/MyLineEdit.cpp \
    MovieWidget.cpp \
    smallWidgets/MyLabel.cpp \
    MovieImageDialog.cpp \
    DownloadManager.cpp \
    FilesWidget.cpp \
    data/MovieModel.cpp \
    data/MovieProxyModel.cpp \
    AboutDialog.cpp \
    scrapers/VideoBuster.cpp \
    scrapers/OFDb.cpp \
    scrapers/Cinefacts.cpp \
    smallWidgets/FilterWidget.cpp \
    smallWidgets/MyTableWidget.cpp \
    MessageBox.cpp \
    Message.cpp \
    data/TvShow.cpp \
    data/TvShowFileSearcher.cpp \
    data/TvShowModel.cpp \
    TvShowFilesWidget.cpp \
    data/TvShowProxyModel.cpp \
    data/TvShowModelItem.cpp \
    data/TvShowEpisode.cpp \
    data/TvShowDelegate.cpp \
    TvShowWidget.cpp \
    TvShowWidgetEpisode.cpp \
    TvShowWidgetTvShow.cpp \
    scrapers/TheTvDb.cpp \
    TvShowSearch.cpp \
    mediaCenterPlugins/XbmcSql.cpp \
    DownloadManagerElement.cpp \
    smallWidgets/ImageLabel.cpp \
    ImagePreviewDialog.cpp \
    SetsWidget.cpp \
    MovieListDialog.cpp \
    Helper.cpp \
    smallWidgets/MyTreeView.cpp \
    data/MovieDelegate.cpp \
    settings/Settings.cpp

HEADERS  += MainWindow.h \
    data/Movie.h \
    Globals.h \
    data/MediaCenterInterface.h \
    data/MovieFileSearcher.h \
    mediaCenterPlugins/XbmcXml.h \
    scrapers/TMDb.h \
    data/ScraperInterface.h \
    Manager.h \
    MovieSearch.h \
    smallWidgets/MyLineEdit.h \
    MovieWidget.h \
    smallWidgets/MyLabel.h \
    MovieImageDialog.h \
    DownloadManager.h \
    FilesWidget.h \
    data/MovieModel.h \
    data/MovieProxyModel.h \
    AboutDialog.h \
    scrapers/VideoBuster.h \
    scrapers/OFDb.h \
    scrapers/Cinefacts.h \
    smallWidgets/FilterWidget.h \
    smallWidgets/MyTableWidget.h \
    MessageBox.h \
    Message.h \
    data/TvShow.h \
    data/TvShowFileSearcher.h \
    data/TvShowModel.h \
    TvShowFilesWidget.h \
    data/TvShowProxyModel.h \
    data/TvShowModelItem.h \
    data/TvShowEpisode.h \
    data/TvShowDelegate.h \
    TvShowWidget.h \
    TvShowWidgetEpisode.h \
    TvShowWidgetTvShow.h \
    scrapers/TheTvDb.h \
    data/TvScraperInterface.h \
    TvShowSearch.h \
    mediaCenterPlugins/XbmcSql.h \
    DownloadManagerElement.h \
    smallWidgets/ImageLabel.h \
    ImagePreviewDialog.h \
    SetsWidget.h \
    MovieListDialog.h \
    Helper.h \
    smallWidgets/MyTreeView.h \
    data/MovieDelegate.h \
    settings/Settings.h

FORMS    += MainWindow.ui \
    MovieSearch.ui \
    MovieWidget.ui \
    MovieImageDialog.ui \
    FilesWidget.ui \
    AboutDialog.ui \
    smallWidgets/FilterWidget.ui \
    MessageBox.ui \
    Message.ui \
    TvShowFilesWidget.ui \
    TvShowWidget.ui \
    TvShowWidgetEpisode.ui \
    TvShowWidgetTvShow.ui \
    TvShowSearch.ui \
    smallWidgets/ImageLabel.ui \
    ImagePreviewDialog.ui \
    SetsWidget.ui \
    MovieListDialog.ui \
    settings/Settings.ui

RESOURCES += \
    MediaElch.qrc

TRANSLATIONS += \
    i18n/MediaElch_de.ts

ICON = MediaElch.icns
RC_FILE = MediaElch.rc
