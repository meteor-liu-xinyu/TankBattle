#-------------------------------------------------
#
# Project created by QtCreator 2025-05-14T14:05:58
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TankBattle
TEMPLATE = app


SOURCES += main.cpp\
        mainwidget.cpp \
    tank.cpp \
    bullet.cpp \
    obstacle.cpp \
    boom.cpp \
    stopwidget.cpp \
    gameoverui.cpp \
    food.cpp

HEADERS  += mainwidget.h \
    entity.h \
    tank.h \
    bullet.h \
    obstacle.h \
    boom.h \
    stopwidget.h \
    gameoverui.h \
    food.h

FORMS    += mainwidget.ui \
    boom.ui \
    stopwidget.ui \
    gameoverui.ui

RESOURCES += \
    resources.qrc
