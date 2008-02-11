# Fontmatrix src/CMakeLists.txt v1

set(fontmatrix_SRCS
aboutwidget.cpp
dataloader.cpp
fmglyphsview.cpp
fmpreviewlist.cpp
fmsampletextview.cpp
# fontactionwidget.cpp
fontbookdialog.cpp
fontitem.cpp
helpwidget.cpp
importedfontsdialog.cpp
listdockwidget.cpp
main.cpp
mainviewwidget.cpp
savedata.cpp
systray.cpp
tagseteditor.cpp
typotek.cpp
fmotf.cpp
prefspaneldialog.cpp
fmshaper.cpp
fontbook.cpp
importtags.cpp
dataexport.cpp
)

SET(fontmatrix_MOC_HDRS
aboutwidget.h
fmglyphsview.h
fmpreviewlist.h
fmsampletextview.h
# fontactionwidget.h
fontbookdialog.h
fontitem.h
helpwidget.h
listdockwidget.h
mainviewwidget.h
systray.h
tagseteditor.h
typotek.h
prefspaneldialog.h
importtags.h
)
QT4_WRAP_CPP(fontmatrix_MOC_SRCS ${fontmatrix_MOC_HDRS})


SET(fontmatrix_UIS
about.ui
bookexport.ui
# fontaction.ui
help.ui
importedfonts.ui
listsdock.ui
mainview.ui
tagset.ui
prefs_panel.ui
importtags.ui
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
