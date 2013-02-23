DEPTH = ../../..
include($${DEPTH}/qjsonrpc.pri)
include($${DEPTH}/tests/tests.pri)

TEMPLATE = app
TARGET = server
HEADERS = testservice.h
SOURCES = testservice.cpp \
          localserver.cpp
