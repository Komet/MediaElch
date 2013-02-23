macx:CONFIG -= app_bundle
CONFIG  += qtestlib
INCLUDEPATH += $${QJSONRPC_INCLUDEPATH} \
               $${QJSONRPC_INCLUDEPATH}/json
LIBS += -L$${DEPTH}/src $${QJSONRPC_LIBS}
