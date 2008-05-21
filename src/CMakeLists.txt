# Fontmatrix src/CMakeLists.txt v1

set(fontmatrix_SRCS
aboutwidget.cpp
dataexport.cpp
dataloader.cpp
fmactivate.cpp
fmbaseshaper.cpp
fmglyphsview.cpp
fmnamelist.cpp
fmotf.cpp
fmplayground.cpp
fmpreviewlist.cpp
# fmprintdialog.cpp
fmrepair.cpp
fmsampletextview.cpp
fmshaper_own.cpp
fontbook.cpp
fontbookdialog.cpp
fontitem.cpp
fontmatrixshaper.cpp
helpwidget.cpp
importedfontsdialog.cpp
importtags.cpp
listdockwidget.cpp
main.cpp
mainviewwidget.cpp
pangoshaper.cpp
prefspaneldialog.cpp
remotedir.cpp
savedata.cpp
shortcuts.cpp
systray.cpp
tagseteditor.cpp
textprogression.cpp
typotek.cpp
)

IF(HARFBUZZSHAPER)
	SET(HARFBUZZ_SRCS 
	harfbuzzqtshaper.cpp
	fmshaper.cpp)
ENDIF(HARFBUZZSHAPER)

IF(ICUSHAPER)
	SET(ICU_SRCS icushaper.cpp)
ENDIF(ICUSHAPER)

IF(M17NSHAPER)
	SET(M17N_SRCS
	m17nshaper.cpp
	)
ENDIF(M17NSHAPER)

SET(SHAPERS_SRCS
${HARFBUZZ_SRCS}
${ICU_SRCS}
${M17N_SRCS}
)

SET(SHAPERS_LIBRARIES
${ICU_LIBRARIES}
${M17N_LIBRARIES}
)

SET(fontmatrix_MOC_HDRS
aboutwidget.h
fmactivate.h
fmglyphsview.h
fmnamelist.h
fmplayground.h
fmpreviewlist.h
# fmprintdialog.h
fmrepair.h
fmsampletextview.h
fontbookdialog.h
fontitem.h
helpwidget.h
importtags.h
listdockwidget.h
mainviewwidget.h
prefspaneldialog.h
remotedir.h
shortcuts.h
systray.h
tagseteditor.h
textprogression.h
typotek.h
)
QT4_WRAP_CPP(fontmatrix_MOC_SRCS ${fontmatrix_MOC_HDRS})


SET(fontmatrix_UIS
about.ui
bookexport.ui
importedfonts.ui
importtags.ui
help.ui
listsdock.ui
mainview.ui
# multiprintdialog.ui
prefs_panel.ui
repair.ui
tagset.ui
textprogression.ui
)
QT4_WRAP_UI(fontmatrix_UIS_H ${fontmatrix_UIS})

SET(fontmatrix_RCCS
application.qrc
)
QT4_ADD_RESOURCES(fontmatrix_RCC_SRCS ${fontmatrix_RCCS})

include_directories(
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_BINARY_DIR}
${QT_INCLUDE_DIR}
${QT_QT_INCLUDE_DIR}
${FREETYPE_INCLUDE_DIRS}
${HARFBUZZ_INCLUDE_DIR}
${FONTCONFIG_INCLUDE_DIR}
${ICU_INCLUDE_DIR}
${M17N_INCLUDE_DIR}
)

add_executable(fontmatrix
${fontmatrix_SRCS}
${SHAPERS_SRCS}
${fontmatrix_MOC_SRCS}
${fontmatrix_UIS_H}
${fontmatrix_RCC_SRCS}
)

IF(SHOW_LIBS)
	MESSAGE(STATUS "Qt4 libs:" ${QT_LIBRARIES})
	MESSAGE(STATUS "FreeType libs: " ${FREETYPE_LIBRARIES})
	MESSAGE(STATUS "ICU libs: " ${ICU_LIBRARIES})
ENDIF(SHOW_LIBS)

target_link_libraries(fontmatrix harfbuzz
${QT_LIBRARIES}
${FREETYPE_LIBRARIES}
${FONTCONFIG_LIBRARIES}
${SHAPERS_LIBRARIES}
)

INSTALL(TARGETS fontmatrix
      RUNTIME DESTINATION bin
)
