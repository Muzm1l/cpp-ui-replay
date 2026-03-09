QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    datparser.cpp \
    eventtablewidget.cpp \
    trajectoryview.cpp

HEADERS += \
    datpacket.h \
    datparser.h \
    parser_format_config.h \
    mainwindow.h \
    eventtablewidget.h \
    trajectoryview.h

# Show non-GUI parser/analyzer/decoder files in Qt Creator project tree
DISTFILES += \
    README.md \
    Makefile.datparser \
    datparser.cpp \
    datparser.h \
    datparser_main.cpp \
    dat_analyzer.cpp \
    dat_analyzer.py \
    packet_decoder.cpp \
    packet_decoder.py

OTHER_FILES += $$DISTFILES

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
