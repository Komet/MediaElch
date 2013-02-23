include(../qjsonrpc.pri)

TEMPLATE = lib
TARGET = qjsonrpc
QT += core network
DEFINES += QJSONRPC_BUILD
CONFIG += $${QJSONRPC_LIBRARY_TYPE}
VERSION = $${QJSONRPC_VERSION}
win32:DESTDIR = $$OUT_PWD

# check if we need to build qjson
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VERSION_MAJOR = $$member(QT_VERSION, 0)
lessThan(QT_VERSION_MAJOR, 5) {
    include(json/json.pri)
}

PRIVATE_HEADERS += \
    qjsonrpcservice_p.h \
    qjsonrpcmessage_p.h \

INSTALL_HEADERS += \
    qjsonrpcservice.h \
    qjsonrpcmessage.h \
    qjsonrpc_export.h

HEADERS += \
    $${INSTALL_HEADERS} \
    $${PRIVATE_HEADERS}
       
SOURCES += \
    qjsonrpcservice.cpp \
    qjsonrpcmessage.cpp


# install
headers.files = $${INSTALL_HEADERS}
headers.path = $${PREFIX}/include/qjsonrpc
target.path = $${PREFIX}/$${LIBDIR}
INSTALLS += headers target

# pkg-config support
CONFIG += create_pc create_prl no_install_prl
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$headers.path
equals(QJSONRPC_LIBRARY_TYPE, staticlib) {
    QMAKE_PKGCONFIG_CFLAGS = -DQJSONRPC_STATIC
} else {
    QMAKE_PKGCONFIG_CFLAGS = -DQJSONRPC_SHARED
}
unix:QMAKE_CLEAN += -r pkgconfig lib$${TARGET}.prl

