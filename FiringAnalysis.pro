QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Specify build directories for generated files
MOC_DIR = build/moc
OBJECTS_DIR = build/obj
UI_DIR = build/ui
RCC_DIR = build/rcc

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    home.cpp \
    parser.cpp \
    parserdialog.cpp \
    eventview.cpp \
    parametrview.cpp \
    trajectoryview.cpp \
    geographicalinfo.cpp \
    acousticpanorama.cpp \
    demongraph.cpp

HEADERS += \
    parser.h \
    parser_format_config.h \
    parserdialog.h \
    home.h \
    eventview.h \
    parametrview.h \
    trajectoryview.h \
    geographicalinfo.h \
    acousticpanorama.h \
    demongraph.h

# Show non-GUI parser/analyzer/decoder files in Qt Creator project tree
DISTFILES += \
    README.md \
    parser.cpp \
    parser.h

OTHER_FILES += $$DISTFILES

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
