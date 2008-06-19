# Fontmatrix src/CMakeLists.txt v1

set(fontmatrix_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/aboutwidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/dataexport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/dataloader.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmactivate.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmbaseshaper.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmglyphsview.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmlayoptwidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmlayout.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmnamelist.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmotf.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmplayground.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmpreviewlist.cpp
# fmprintdialog.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmrepair.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmsampletextview.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmshaper_own.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmverticalbutton.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fontbook.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fontbookdialog.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fontitem.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fontmatrixshaper.cpp
${CMAKE_CURRENT_SOURCE_DIR}/helpwidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/importedfontsdialog.cpp
${CMAKE_CURRENT_SOURCE_DIR}/importtags.cpp
${CMAKE_CURRENT_SOURCE_DIR}/listdockwidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
${CMAKE_CURRENT_SOURCE_DIR}/mainviewwidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/pangoshaper.cpp
${CMAKE_CURRENT_SOURCE_DIR}/prefspaneldialog.cpp
${CMAKE_CURRENT_SOURCE_DIR}/remotedir.cpp
${CMAKE_CURRENT_SOURCE_DIR}/savedata.cpp
${CMAKE_CURRENT_SOURCE_DIR}/shortcuts.cpp
${CMAKE_CURRENT_SOURCE_DIR}/systray.cpp
${CMAKE_CURRENT_SOURCE_DIR}/tagseteditor.cpp
${CMAKE_CURRENT_SOURCE_DIR}/textprogression.cpp
${CMAKE_CURRENT_SOURCE_DIR}/typotek.cpp
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
fmlayoptwidget.h
fmlayout.h
fmnamelist.h
fmplayground.h
fmpreviewlist.h
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
layoutoptions.ui
listsdock.ui
mainview.ui
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

ADD_SUBDIRECTORY(messages)

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
${HYPHENATE_INCLUDE_DIR}
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

target_link_libraries(fontmatrix harfbuzz hyphenate
${QT_LIBRARIES}
${FREETYPE_LIBRARIES}
${FONTCONFIG_LIBRARIES}
${SHAPERS_LIBRARIES}
)

INSTALL(TARGETS fontmatrix
      RUNTIME DESTINATION bin
)


