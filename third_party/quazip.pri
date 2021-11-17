INCLUDEPATH += $$PWD/quazip/quazip
DEPENDPATH += $$PWD/quazip/quazip

HEADERS += \
        $$PWD/quazip/quazip/JlCompress.h \
        $$PWD/quazip/quazip/ioapi.h \
        $$PWD/quazip/quazip/minizip_crypt.h \
        $$PWD/quazip/quazip/quaadler32.h \
        $$PWD/quazip/quazip/quachecksum32.h \
        $$PWD/quazip/quazip/quacrc32.h \
        $$PWD/quazip/quazip/quagzipfile.h \
        $$PWD/quazip/quazip/quaziodevice.h \
        $$PWD/quazip/quazip/quazip.h \
        $$PWD/quazip/quazip/quazip_global.h \
        $$PWD/quazip/quazip/quazip_qt_compat.h \
        $$PWD/quazip/quazip/quazipdir.h \
        $$PWD/quazip/quazip/quazipfile.h \
        $$PWD/quazip/quazip/quazipfileinfo.h \
        $$PWD/quazip/quazip/quazipnewinfo.h \
        $$PWD/quazip/quazip/unzip.h \
        $$PWD/quazip/quazip/zip.h

SOURCES += \
        $$PWD/quazip/quazip/unzip.c \
        $$PWD/quazip/quazip/zip.c \
        $$PWD/quazip/quazip/JlCompress.cpp \
        $$PWD/quazip/quazip/qioapi.cpp \
        $$PWD/quazip/quazip/quaadler32.cpp \
        $$PWD/quazip/quazip/quachecksum32.cpp \
        $$PWD/quazip/quazip/quacrc32.cpp \
        $$PWD/quazip/quazip/quagzipfile.cpp \
        $$PWD/quazip/quazip/quaziodevice.cpp \
        $$PWD/quazip/quazip/quazip.cpp \
        $$PWD/quazip/quazip/quazipdir.cpp \
        $$PWD/quazip/quazip/quazipfile.cpp \
        $$PWD/quazip/quazip/quazipfileinfo.cpp \
        $$PWD/quazip/quazip/quazipnewinfo.cpp
