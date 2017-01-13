QT += core gui xml network widgets multimedia location

TARGET = GeoGallery
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    networkrequestmanager.cpp \
    imagemodel.cpp \
    imagelog.cpp \
    exifreader.cpp \
    openurldialog.cpp \
    gpxreader.cpp

HEADERS  += mainwindow.h \
    main.h \
    networkrequestmanager.h \
    imagemodel.h \
    imagelog.h \
    exifreader.h \
    openurldialog.h \
    gpxreader.h

FORMS    += mainwindow.ui \
    openurldialog.ui

RESOURCES += \
    resources.qrc


