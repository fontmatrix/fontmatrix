SOURCES += typotek.cpp \
           main.cpp \
 mainviewwidget.cpp \
 fontitem.cpp \
 fontactionwidget.cpp \
 typotekadaptator.cpp \
 fontbookdialog.cpp \
 dataloader.cpp \
 tagseteditor.cpp \
 savedata.cpp \
fmsampletextview.cpp
 
HEADERS += typotek.h \
 mainviewwidget.h \
 fontitem.h \
 fontactionwidget.h \
 typotekadaptator.h \
 fontbookdialog.h \
 dataloader.h \
 tagseteditor.h \
 savedata.h \
 fmsampletextview.h
 
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt \
 uitools \
 qdbus \
 debug
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

desktop.files = ../fontmatrix.desktop
desktop.path = /usr/local/share/applications


INSTALLS += desktop

target.path = /usr/local/bin

TARGET = ../bin/fontmatrix


icons.files = ./icons/application-fontmatrix_16.png \
 application-fontmatrix_22.png \ 
 application-fontmatrix_32.png \
 application-fontmatrix_48.png \ 
 application-fontmatrix_64.png \
 application-fontmatrix_126.png
icons.path = /usr/local/share/fontmatrix/icons/

INSTALLS += icons
