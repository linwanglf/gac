######################################################################
# Automatically generated by qmake (3.0) Wed May 31 17:02:43 2017
######################################################################

QT +=widgets sql
TEMPLATE = app
TARGET = gac
INCLUDEPATH += . UI qextserial database

# Input
HEADERS += comvar.h \
           logindialog.h \
           mainwindow.h \
           parasetting.h \
           qsg3fquerymodel.h \
           thread.h \
           database/connect.h \
           qextserial/qextserialport.h \
           qextserial/qextserialport_global.h \
           qextserial/qextserialport_p.h \
           UI/ui_logindialog.h \
           UI/ui_mainwindow.h
FORMS += logindialog.ui mainwindow.ui parasetting.ui
SOURCES += logindialog.cpp \
           main.cpp \
           mainwindow.cpp \
           parasetting.cpp \
           qsg3fquerymodel.cpp \
           thread.cpp \
           qextserial/qextserialport.cpp \
           qextserial/qextserialport_unix.cpp 
