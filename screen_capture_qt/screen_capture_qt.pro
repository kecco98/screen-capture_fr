QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++2a

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ScreenCapture.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    ScreenCapture.h \
    mainwindow.h

FORMS += \
    mainwindow.ui


CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

linux:LIBS += -lavformat -lavcodec -lavutil -lavdevice -lswscale -lX11 -lpthread -lswresample


win32:INCLUDEPATH += $$PWD/../include
win32:DEPENDPATH += $$PWD/../include

