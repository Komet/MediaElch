#-------------------------------------------------
#
# Project created by QtCreator 2012-02-16T11:13:07
#
#-------------------------------------------------

QT       += core gui network script xml

TARGET = XBMM
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    SettingsDialog.cpp \
    data/Movie.cpp \
    data/MovieFileSearcher.cpp \
    mediaCenterPlugins/XbmcXml.cpp \
    scrapers/TMDb.cpp \
    Manager.cpp \
    MovieSearch.cpp \
    MyLineEdit.cpp \
    MovieWidget.cpp \
    MyLabel.cpp \
    MovieImageDialog.cpp \
    DownloadManager.cpp \
    FilesWidget.cpp \
    data/MovieModel.cpp \
    data/MovieProxyModel.cpp \
    AboutDialog.cpp \
    MyTableView.cpp \
    MyGroupBox.cpp \
    scrapers/VideoBuster.cpp \
    scrapers/OFDb.cpp \
    scrapers/Cinefacts.cpp \
    QuestionDialog.cpp \
    smallWidgets/FilterWidget.cpp \
    MyTableWidget.cpp \
    ExportDialog.cpp

HEADERS  += MainWindow.h \
    SettingsDialog.h \
    data/Movie.h \
    Globals.h \
    data/MediaCenterInterface.h \
    data/MovieFileSearcher.h \
    mediaCenterPlugins/XbmcXml.h \
    scrapers/TMDb.h \
    data/ScraperInterface.h \
    Manager.h \
    MovieSearch.h \
    MyLineEdit.h \
    MovieWidget.h \
    MyLabel.h \
    MovieImageDialog.h \
    DownloadManager.h \
    FilesWidget.h \
    data/MovieModel.h \
    data/MovieProxyModel.h \
    AboutDialog.h \
    MyTableView.h \
    MyGroupBox.h \
    scrapers/VideoBuster.h \
    scrapers/OFDb.h \
    scrapers/Cinefacts.h \
    QuestionDialog.h \
    smallWidgets/FilterWidget.h \
    MyTableWidget.h \
    ExportDialog.h

FORMS    += MainWindow.ui \
    SettingsDialog.ui \
    MovieSearch.ui \
    MovieWidget.ui \
    MovieImageDialog.ui \
    FilesWidget.ui \
    AboutDialog.ui \
    QuestionDialog.ui \
    smallWidgets/FilterWidget.ui \
    ExportDialog.ui

RESOURCES += \
    XBMM.qrc

TRANSLATIONS += \
    i18n/XBMM_de.ts

ICON = XBMM.icns
RC_FILE = XBMM.rc
