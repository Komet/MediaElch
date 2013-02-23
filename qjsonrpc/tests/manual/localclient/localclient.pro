DEPTH = ../../..
include($${DEPTH}/qjsonrpc.pri)
include($${DEPTH}/tests/tests.pri)

TEMPLATE = app
HEADERS = localclient.h
SOURCES = localclient.cpp \
          main.cpp

