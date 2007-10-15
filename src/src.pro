SOURCES += typotek.cpp \
           main.cpp \
 mainviewwidget.cpp \
 fontitem.cpp \
 fontactionwidget.cpp
HEADERS += typotek.h \
 mainviewwidget.h \
 fontitem.h \
 fontactionwidget.h
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt \
 uitools
TARGET = ../bin/typotek
RESOURCES = application.qrc
FORMS += mainview.ui \
 fontaction.ui

LIBS += -L/usr/lib \
-lfreetype
INCLUDEPATH += /usr/include/freetype2


INSTALLS += target

target.path = /usr/bin

