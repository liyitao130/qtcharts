!include( ../tests.pri ) {
    error( "Couldn't find the test.pri file!" )
}

RESOURCES += resources.qrc
SOURCES += main.cpp

include(qmlapplicationviewer/qmlapplicationviewer.pri)
