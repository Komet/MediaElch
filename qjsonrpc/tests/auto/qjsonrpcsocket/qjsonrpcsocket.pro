load(qttest_p4)
DEPTH = ../../..
include($${DEPTH}/qjsonrpc.pri)
include($${DEPTH}/tests/tests.pri)

TARGET = tst_qjsonrpcsocket
SOURCES = tst_qjsonrpcsocket.cpp
RESOURCES = tst_qjsonrpcsocket.qrc
