# -------------------------------------------------
# Project created by QtCreator 2011-04-01T11:38:12
# -------------------------------------------------

# QT -= core gui, means that we should not link the default qt libs into the component
# Template = lib, means that we want to build a library (dll)
QT -= core gui
TEMPLATE = lib

# The name of the compiled dll, (.dll will be added automatically)
TARGET = exampleLib #Change this. This is where you put your name of the library target file.

# Destination for the compiled dll. $${PWD}/ means the same directory as this .pro file, even if you use shadow build
DESTDIR = $${PWD}/

# A location to search for the Hopsan include files, by specifying the path here, you dont need to do this everywhere in all of your component hpp files
# You can also add additional paths for eg. your own Utility functions
INCLUDEPATH *= $${PWD}/../../HopsanCore

# The location of the HopsanCore dll file, needed to link against when compiling your library
LIBS *= -L$${PWD}/../../bin

#Special options for deug and release mode
CONFIG(debug, debug|release) {
    LIBS *= -lHopsanCore_d #In debug mode HopsanCore has the debug extension _d
}
CONFIG(release, debug|release) {
    LIBS *= -lHopsanCore
}

# -------------------------------------------------
# Project files
# -------------------------------------------------
SOURCES += \
    exampleLib.cc

HEADERS += \
    component_code/myWickedOrifice.hpp \
    component_code/myWickedVolume.hpp

OTHER_FILES +=
