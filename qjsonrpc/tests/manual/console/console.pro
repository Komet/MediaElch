DEPTH = ../../..
include($${DEPTH}/qjsonrpc.pri)
include($${DEPTH}/tests/tests.pri)

TEMPLATE = app
QT += script core
HEADERS = interface.h
SOURCES = interface.cpp \
          main.cpp
