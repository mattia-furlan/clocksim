#-------------------------------------------------
#
# Project created by QtCreator 2016-08-09T09:57:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = clocksim
TEMPLATE = app


SOURCES += main.cpp\
        clocksim.cpp \
    clock.cpp \
    settingsform.cpp \
    qlineedithotkey.cpp \
    pbsviewer.cpp \
    qclickablelabel.cpp \
    qcolorselector.cpp \
    qtablewidgetitemcopyable.cpp \
    structures.cpp

HEADERS  += \
    clock.h \
    clocksim.h \
    settingsform.h \
    qlineedithotkey.h \
    pbsviewer.h \
    qclickablelabel.h \
    qcolorselector.h \
    qtablewidgetitemcopyable.h \
    structures.h

FORMS    += clocksim.ui \
    settingsform.ui \
    pbsviewer.ui

windows
{
    RC_FILE = clocksim.rc
}

macx
{
     ICON = clocksim.icns
}

CONFIG += c++14

DISTFILES += \
    TODO.txt \
    README.txt \
    COPYING.txt


