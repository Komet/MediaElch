#!/bin/sh
#qtchooser -list-versions
#export QT_SELECT=qt5
#
#set -evx

#env | sort

qmake -v
qmake
make
