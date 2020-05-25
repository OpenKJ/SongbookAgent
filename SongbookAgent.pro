#-------------------------------------------------
#
# Project created by QtCreator 2018-06-06T10:57:25
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network

win32: RC_ICONS = resources/AppIcon.ico

TARGET = SongbookAgent
TEMPLATE = app
QMAKE_TARGET_COMPANY = The OpenKJ Project
QMAKE_TARGET_PRODUCT = Songbook Agent
QMAKE_TARGET_DESCRIPTION = OpenKJ Songbook Standalone Agent
QMAKE_TARGET_BUNDLE_PREFIX = com.okjsongbook

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

DEFINES += BUILD_DATE=__DATE__

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

macx: {
	ICON = resources/SongbookAgent.icns
}

SOURCES += \
        main.cpp \
        songbookclient.cpp \
    settings.cpp \
    dialogsettings.cpp \
    okjsongbookapi.cpp \
    requeststablemodel.cpp \
    dialogupdate.cpp \
    dialogabout.cpp \
    dialogupdater.cpp

HEADERS += \
        songbookclient.h \
    settings.h \
    dialogsettings.h \
    okjsongbookapi.h \
    requeststablemodel.h \
    dialogupdate.h \
    dialogabout.h \
    dialogupdater.h

FORMS += \
        songbookclient.ui \
    dialogsettings.ui \
    dialogupdate.ui \
    dialogabout.ui \
    dialogupdater.ui

RESOURCES += \
    resources.qrc
