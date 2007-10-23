SOURCES += typotek.cpp \
           main.cpp \
 mainviewwidget.cpp \
 fontitem.cpp \
 fontactionwidget.cpp \
 typotekadaptator.cpp \
 fontbookdialog.cpp
HEADERS += typotek.h \
 mainviewwidget.h \
 fontitem.h \
 fontactionwidget.h \
 typotekadaptator.h \
 fontbookdialog.h
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt \
 uitools \
 qdbus \
 build_all \
 debug
TARGET = ../bin/typotek
RESOURCES = application.qrc
FORMS += mainview.ui \
 fontaction.ui \
 bookexport.ui

LIBS += -L/usr/lib \
-lfreetype
INCLUDEPATH += /usr/include/freetype2


INSTALLS += target

target.path = /usr/bin


CONFIG -= release

