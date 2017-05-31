#!/bin/sh
source /etc/profile
qmakearm5.2 -project "QT +=widgets sql"
qmakearm5.2
make
make clean
