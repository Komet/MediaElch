load(qttest_p4)
DEPTH = ../../..
include($${DEPTH}/qjsonrpc.pri)
include($${DEPTH}/tests/tests.pri)

TARGET = tst_qjsonrpcserver
SOURCES = tst_qjsonrpcserver.cpp
