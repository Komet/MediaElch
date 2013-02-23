QJSONRPC_VERSION = 1.0.0

isEmpty(QJSONRPC_LIBRARY_TYPE) {
    QJSONRPC_LIBRARY_TYPE = shared
}

QT += network
QJSONRPC_INCLUDEPATH = $${PWD}/src
QJSONRPC_LIBS = -lqjsonrpc
contains(QJSONRPC_LIBRARY_TYPE, staticlib) {
    DEFINES += QJSONRPC_STATIC
} else {
    DEFINES += QJSONRPC_SHARED
    win32:QJSONRPC_LIBS = -lqjsonrpc1
}

isEmpty(PREFIX) {
    unix {
        PREFIX = /usr/local
    } else {
        PREFIX = $$[QT_INSTALL_PREFIX]
    }
}
isEmpty(LIBDIR) {
    LIBDIR = lib
}

