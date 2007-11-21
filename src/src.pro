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
fmsampletextview.cpp \
 fmpreviewlist.cpp

HEADERS += typotek.h \
 mainviewwidget.h \
 fontitem.h \
 fontactionwidget.h \
 typotekadaptator.h \
 fontbookdialog.h \
 dataloader.h \
 tagseteditor.h \
 savedata.h \
 fmsampletextview.h \
 fmpreviewlist.h

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
desktop.path = /usr/share/applications


INSTALLS += desktop

target.path = /usr/bin

TARGET = ../bin/fontmatrix

#VLADA: This is the pattern I should follow for this application to be visible in both GNOME and KDE (possibly other WMs); right now the only installed icon is 48x48

icons.files = ../fontmatrix.png
icons.path = /usr/share/icons/hicolor/48x48/apps

INSTALLS += icons
