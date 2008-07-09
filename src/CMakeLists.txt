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
${CMAKE_CURRENT_SOURCE_DIR}/fmpaths.cpp
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
${CMAKE_CURRENT_SOURCE_DIR}/about.ui
${CMAKE_CURRENT_SOURCE_DIR}/bookexport.ui
${CMAKE_CURRENT_SOURCE_DIR}/importedfonts.ui
${CMAKE_CURRENT_SOURCE_DIR}/importtags.ui
${CMAKE_CURRENT_SOURCE_DIR}/help.ui
${CMAKE_CURRENT_SOURCE_DIR}/layoutoptions.ui
${CMAKE_CURRENT_SOURCE_DIR}/listsdock.ui
${CMAKE_CURRENT_SOURCE_DIR}/mainview.ui
${CMAKE_CURRENT_SOURCE_DIR}/prefs_panel.ui
${CMAKE_CURRENT_SOURCE_DIR}/repair.ui
${CMAKE_CURRENT_SOURCE_DIR}/tagset.ui
${CMAKE_CURRENT_SOURCE_DIR}/textprogression.ui
)
QT4_WRAP_UI(fontmatrix_UIS_H ${fontmatrix_UIS})


SET(fontmatrix_RCCS
application.qrc
)
QT4_ADD_RESOURCES(fontmatrix_RCC_SRCS ${fontmatrix_RCCS})

SET ( fontmatrix_TSS
      messages/fontmatrix-da.ts
      messages/fontmatrix-de.ts
      messages/fontmatrix-en.ts
      messages/fontmatrix-fi.ts
      messages/fontmatrix-fr.ts
      messages/fontmatrix-hi.ts
      messages/fontmatrix-nb.ts
      messages/fontmatrix-nl.ts
      messages/fontmatrix-no.ts
      messages/fontmatrix-ru.ts
      messages/fontmatrix-sr.ts
      messages/fontmatrix-sv.ts
      messages/fontmatrix-uk.ts
      messages/fontmatrix-zh_CN.ts
      messages/fontmatrix-zh_TW.ts
    )

IF ( TRANSLATOR )
	MESSAGE(STATUS "We are glad to see you handle the fontmatrix-${TRANSLATOR}.ts file, thank you very much")
	add_custom_command(TARGET translation
 			PRE_BUILD 
			COMMAND ${QT_LUPDATE_EXECUTABLE} ARGS ${fontmatrix_SRCS} ${fontmatrix_UIS} -ts "fontmatrix-${TRANSLATOR}.ts")
ENDIF( TRANSLATOR )
QT4_ADD_TRANSLATION ( fontmatrix_QMS ${fontmatrix_TSS} )
ADD_CUSTOM_TARGET ( translation ALL DEPENDS ${fontmatrix_QMS} )

# ADD_SUBDIRECTORY(messages)

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


# IF(SHOW_LIBS)
# 	MESSAGE(STATUS "Qt4 libs:" ${QT_LIBRARIES})
# 	MESSAGE(STATUS "FreeType libs: " ${FREETYPE_LIBRARIES})
# 	MESSAGE(STATUS "ICU libs: " ${ICU_LIBRARIES})
# ENDIF(SHOW_LIBS)

IF(UNIX AND NOT APPLE)
	add_executable(fontmatrix
	${fontmatrix_SRCS}
	${SHAPERS_SRCS}
	${fontmatrix_MOC_SRCS}
	${fontmatrix_UIS_H}
	${fontmatrix_RCC_SRCS}
	)

	target_link_libraries(fontmatrix harfbuzz hyphenate
	${QT_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${FONTCONFIG_LIBRARIES}
	${SHAPERS_LIBRARIES}
	)
	
	INSTALL(TARGETS fontmatrix
	RUNTIME DESTINATION ${FONTMATRIX_BIN}
	)
	INSTALL ( FILES ${fontmatrix_QMS} DESTINATION ${FONTMATRIX_QMDIR} )
ENDIF(UNIX AND NOT APPLE)

IF(APPLE)
	add_executable(fontmatrix MACOSX_BUNDLE
	${fontmatrix_SRCS}
	${SHAPERS_SRCS}
	${fontmatrix_MOC_SRCS}
	${fontmatrix_UIS_H}
	${fontmatrix_RCC_SRCS}
	)
	
	target_link_libraries(fontmatrix harfbuzz hyphenate
	${QT_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${SHAPERS_LIBRARIES}
	)

	# here we’ll poupulate the bundle in order to make it
	# self containing regarding external libs (Qt4 & Freetype)
	# and resources such as QM files.
	# NOTE: Mac users are not expected to build Fontmatrix
	# themselves, thus paths are hard coded because it works on the Mac
	# where we use to build it, period.
	
	SET(PROGNAME fontmatrix)
	SET_SOURCE_FILES_PROPERTIES( fontmatrix PROPERTIES MACOSX_PACKAGE_LOCATION MacOSX )
	SET_SOURCE_FILES_PROPERTIES( ${fontmatrix_QMS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
	SET(MACOSX_BUNDLE_ICON_FILE "application-fontmatrix_128.png")
	SET(MACOSX_BUNDLE_GUI_IDENTIFIER "org.undertype.fontmatrix")
	SET(MACOSX_BUNDLE_NAME "fontmatrix")
	
	
	ADD_CUSTOM_COMMAND( TARGET ${PROGNAME} POST_BUILD
	COMMAND cp ARGS  "${CMAKE_SOURCE_DIR}/src/icons/application-fontmatrix_128.png" ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/
	)
	
	ADD_CUSTOM_COMMAND( TARGET ${PROGNAME} POST_BUILD
	COMMAND cp ARGS ${FM_TEMPLATES_FILES} ${FM_STYLE_FILES} ${FM_WELCOME_FILES} ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources/
	)
	
	ADD_CUSTOM_COMMAND( TARGET translation POST_BUILD
        COMMAND mkdir ARGS -p ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources/locale
        COMMAND cp ARGS ${fontmatrix_QMS} ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources/locale/
	)
	
	ADD_CUSTOM_COMMAND( TARGET ${PROGNAME} POST_BUILD
        COMMAND mkdir ARGS -p ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources
        COMMAND mkdir ARGS -p ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources/lib
        COMMAND mkdir ARGS -p ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources/Frameworks
        COMMAND cp ARGS ${FREETYPE_LIBRARIES} ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources/lib/
        COMMAND rsync ARGS -av --exclude 'Headers/*' /Library/Frameworks/QtCore.framework ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources/Frameworks
        COMMAND rsync ARGS -av --exclude 'Headers/*' /Library/Frameworks/QtGui.framework ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources/Frameworks
        COMMAND rsync ARGS -av --exclude 'Headers/*' /Library/Frameworks/QtXml.framework ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources/Frameworks
        COMMAND rsync ARGS -av --exclude 'Headers/*' /Library/Frameworks/QtSvg.framework ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources/Frameworks
        COMMAND rsync ARGS -av --exclude 'Headers/*' /Library/Frameworks/QtNetwork.framework ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources/Frameworks
        COMMAND rsync ARGS -av --exclude 'Headers/*' /Library/Frameworks/QtWebKit.framework ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources/Frameworks
        COMMAND rsync ARGS -av --exclude 'Headers/*' /Library/Frameworks/QtOpenGL.framework ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources/Frameworks
       )

ENDIF(APPLE)

IF(WIN32)
	add_executable(fontmatrix WIN32
	${fontmatrix_SRCS}
	${SHAPERS_SRCS}
	${fontmatrix_MOC_SRCS}
	${fontmatrix_UIS_H}
	${fontmatrix_RCC_SRCS}
	)

	target_link_libraries(fontmatrix harfbuzz hyphenate
	${QT_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${SHAPERS_LIBRARIES}
	)
	
	INSTALL(TARGETS fontmatrix
	RUNTIME DESTINATION ${FONTMATRIX_BIN}
	)
	INSTALL ( FILES ${fontmatrix_QMS} DESTINATION ${FONTMATRIX_QMDIR} )
ENDIF(WIN32)
