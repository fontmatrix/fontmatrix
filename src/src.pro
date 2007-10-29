SOURCES += typotek.cpp \
           main.cpp \
 mainviewwidget.cpp \
 fontitem.cpp \
 fontactionwidget.cpp \
 typotekadaptator.cpp \
 fontbookdialog.cpp \
 dataloader.cpp \
 tagseteditor.cpp
HEADERS += typotek.h \
 mainviewwidget.h \
 fontitem.h \
 fontactionwidget.h \
 typotekadaptator.h \
 fontbookdialog.h \
 dataloader.h \
 tagseteditor.h
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt \
 uitools \
 qdbus \
 debug
TARGET = ../bin/typotek
RESOURCES = application.qrc
FORMS += mainview.ui \
 fontaction.ui \
 bookexport.ui \
 tagset.ui

LIBS += -L/usr/lib \
-lfreetype
INCLUDEPATH += /usr/include/freetype2


INSTALLS += target



CONFIG -= release

desktop.files = ../typotek.desktop
desktop.path = /usr/share/applications
INSTALLS += desktop

target.path = /usr/local/bin

