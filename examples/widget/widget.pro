TEMPLATE = app
TARGET = Widget
QT += widgets
CONFIG -= embed_manifest_exe
HEADERS += widget.h
SOURCES += widget.cpp main.cpp
win32: RC_FILE = ../example.rc
include(../../qmake/core.pri)
include(../../qmake/widgets.pri)
