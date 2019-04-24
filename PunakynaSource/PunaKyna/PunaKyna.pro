#-------------------------------------------------
#
# Project created by QtCreator 2017-04-19T09:40:44
#
#-------------------------------------------------

# App Versioning
VERSION_VERY_VERY_MINOR = 1
VERSION_VERY_MINOR = 0
VERSION_MINOR = 8
VERSION_MAJOR = 1
VERSION_STATUS = BETA

DEFINES +=  "VERSION_VERY_MINOR=$$VERSION_VERY_MINOR"\
            "VERSION_MINOR=$$VERSION_MINOR"\
            "VERSION_MAJOR=$$VERSION_MAJOR"\
            "VERSION_STATUS=$$VERSION_STATUS"\
            "VERSION_VERY_VERY_MINOR=$$VERSION_VERY_VERY_MINOR"

MAIN_VERSION = v$${VERSION_MAJOR}.$${VERSION_MINOR}
greaterThan(VERSION_MINOR, 0) {
    MAIN_VERSION = $${MAIN_VERSION}.$${VERSION_VERY_MINOR}
    greaterThan(VERSION_VERY_VERY_MINOR, 0) {
        MAIN_VERSION = $${MAIN_VERSION}.$${VERSION_VERY_VERY_MINOR}
    }
}
APPVERSION = "$${MAIN_VERSION}_$${VERSION_STATUS}"

# To better distinguish between release and debug versions of built app...
CONFIG(debug, debug|release) {
    TARGET = "../../PunaKynaDebug/PunaKyna_$${APPVERSION}_Debug"
    DEFINES += "DEBUGG"
} else {
    TARGET = "../../PunaKynaRelease/PunaKyna_$$APPVERSION"
}


# Other defines
DEFINES +=  "BUILD_DEF"

# QTs defines
QT       += core gui pdf xlsx

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

QMAKE_RESOURCE_FLAGS += -compress 0

INCLUDEPATH += ../MuutOhjelmapalat/

SOURCES += main.cpp\
        mainwindow.cpp \
    browserwindow.cpp \
    gradingelement.cpp \
    ../MuutOhjelmapalat/csvparser.cpp \
    pdfwindow.cpp \
    pagerenderer.cpp \
    browsermodel.cpp \
    gradingelementwindow.cpp \
    settings.cpp \
    ../MuutOhjelmapalat/binaryparser.cpp \
    conflictresolver.cpp \
    browserviewitem.cpp \
    mathcheckhandler.cpp \
    displaywidget.cpp \
    csvversionresolver.cpp \
    gradingwindow.cpp \
    autocommenter.cpp \
    smalldialogs.cpp \
    settingswindow.cpp \
    gradingprinter.cpp \
    commentediting.cpp \
    ../MuutOhjelmapalat/textparser.cpp \
#    lsaclasser.cpp \
 #   ../MuutOhjelmapalat/matrix.cpp

HEADERS  +=     build_def.h \
    mainwindow.h \
    browserwindow.h \
    gradingelement.h \
    ../MuutOhjelmapalat/csvparser.h \
    pdfwindow.h \
    pagerenderer.h \
    browsermodel.h \
    gradingelementwindow.h \
    settings.h \
    ../MuutOhjelmapalat/binaryparser.h \
    conflictresolver.h \
    browserviewitem.h \
    mathcheckhandler.h \
    displaywidget.h \
    csvversionresolver.h \
    gradingwindow.h \
    autocommenter.h \
    smalldialogs.h \
    settingswindow.h \
    gradingprinter.h \
    commentediting.h \
    ../MuutOhjelmapalat/textparser.h \
 #   lsaclasser.h \
  #  ../MuutOhjelmapalat/matrix.h

RESOURCES += icons.qrc
