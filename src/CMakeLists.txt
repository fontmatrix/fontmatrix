# Fontmatrix src/CMakeLists.txt v1

set(fontmatrix_SRCS
aboutwidget.cpp
dataexport.cpp
dataloader.cpp
fmglyphsview.cpp
fmnamelist.cpp
fmotf.cpp
fmplayground.cpp
fmpreviewlist.cpp
fmprintdialog.cpp
fmrepair.cpp
fmsampletextview.cpp
fontbook.cpp
fontbookdialog.cpp
fontitem.cpp
helpwidget.cpp
importedfontsdialog.cpp
importtags.cpp
listdockwidget.cpp
main.cpp
mainviewwidget.cpp
prefspaneldialog.cpp
remotedir.cpp
savedata.cpp
systray.cpp
tagseteditor.cpp
textprogression.cpp
typotek.cpp
)

IF(OWN_SHAPER)
	SET(shaper_SRCS
	fmshaper_own.cpp
	)
ELSE(OWN_SHAPER)
	SET(shaper_SRCS
	fmshaper.cpp
	)
ENDIF(OWN_SHAPER)

SET(fontmatrix_MOC_HDRS
aboutwidget.h
fmglyphsview.h
fmnamelist.h
fmplayground.h
fmpreviewlist.h
fmprintdialog.h
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
multiprintdialog.ui
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
${FREETYPE_INCLUDE_DIR}
${HARFBUZZ_INCLUDE_DIR}
${FONTCONFIG_INCLUDE_DIR}
)

add_executable(fontmatrix 
${fontmatrix_SRCS} 
${shaper_SRCS}
${fontmatrix_MOC_SRCS} 
${fontmatrix_UIS_H}
${fontmatrix_RCC_SRCS}
)

# MESSAGE(STATUS ${QT_LIBRARIES})
# MESSAGE(STATUS ${FREETYPE_LIBRARIES})
# MESSAGE(STATUS ${HARFBUZZ_LIBRARIES})

target_link_libraries(fontmatrix harfbuzz
${QT_LIBRARIES}
${FREETYPE_LIBRARIES}
${FONTCONFIG_LIBRARIES}
)

INSTALL(TARGETS fontmatrix
      RUNTIME DESTINATION bin
)
