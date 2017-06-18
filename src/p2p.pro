#-------------------------------------------------
#
# Project created by QtCreator 2017-05-29T20:23:48
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = p2pchat-qt
TEMPLATE = app

DEPENDPATH += .
INCLUDEPATH += .

RC_FILE += resources/icons/main.rc
TRANSLATIONS += zh-cn.ts
# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    tools.cpp

HEADERS  += mainwindow.h \
    tools.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources/resource.qrc

DISTFILES +=

icons.files = resources/p2pchat-qt.png
desktop.files = p2pchat-qt.desktop

isEmpty(INSTALL_PREFIX) {
    unix: INSTALL_PREFIX = /usr
    else: INSTALL_PREFIX = ..
}

unix: {
    desktop.path = $$INSTALL_PREFIX/share/applications
    icons.path = $$INSTALL_PREFIX/share/icons/hicolor/128x128/apps
    INSTALLS += desktop icons
 }

targer.files = p2pchat-qt
target.path = $$INSTALL_PREFIX/bin

INSTALLS += target
