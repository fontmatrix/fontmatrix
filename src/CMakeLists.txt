# Fontmatrix src/CMakeLists.txt v1

set(fontmatrix_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/aboutwidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/dataexport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/dataloader.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmactivate.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmbaseshaper.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmfontdb.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmfontstrings.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmglyphhighlight.cpp
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
fmglyphhighlight.h
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
	ADD_CUSTOM_TARGET(translation 
			COMMAND ${QT_LUPDATE_EXECUTABLE}  ${fontmatrix_SRCS} ${fontmatrix_UIS} -ts "fontmatrix-${TRANSLATOR}.ts"
			DEPENDS ${fontmatrix_SRCS} ${fontmatrix_UIS}
			WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/messages"
			)
ENDIF( TRANSLATOR )
QT4_ADD_TRANSLATION ( fontmatrix_QMS ${fontmatrix_TSS} )
ADD_CUSTOM_TARGET ( translation_release ALL DEPENDS ${fontmatrix_QMS} )

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
	
	# See:
	# http://doc.trolltech.com/4.4/deployment-mac.html
	# http://developer.apple.com/documentation/CoreFoundation/Conceptual/CFBundles/CFBundles.html
	
	SET(PROGNAME fontmatrix)
	
	SET(MACOSX_BUNDLE_BUNDLE_NAME "fontmatrix")
	SET(MACOSX_BUNDLE_ICON_FILE "fontmatrix.icns")
	SET(MACOSX_BUNDLE_GUI_IDENTIFIER "org.undertype.fontmatrix")
	SET(MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
	SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}")
	SET(MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
	SET(MACOSX_BUNDLE_COPYRIGHT "© Fontmatrix team - 2007, 2008")
	
	SET(BUNDLE_RESOURCES_DIR	"${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources/")
	SET(BUNDLE_LOCALES_DIR		"${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources/Locales/")
	
# bundles general resources
	ADD_CUSTOM_COMMAND( TARGET ${PROGNAME} POST_BUILD
	COMMAND mkdir ARGS -p ${BUNDLE_RESOURCES_DIR}
	COMMAND cp ARGS  "${CMAKE_SOURCE_DIR}/src/graphic-resources/fontmatrix.icns" ${BUNDLE_RESOURCES_DIR}
	COMMAND cp ARGS ${FM_TEMPLATES_FILES} ${FM_STYLE_FILES} ${FM_WELCOME_FILES} ${FM_HELP_FILES} ${BUNDLE_RESOURCES_DIR}
	)
# bundles QM files
	ADD_CUSTOM_COMMAND( TARGET translation POST_BUILD
        COMMAND mkdir ARGS -p ${BUNDLE_LOCALES_DIR}
        COMMAND cp ARGS ${fontmatrix_QMS} ${BUNDLE_LOCALES_DIR}
	)
# bundles libs
	ADD_CUSTOM_COMMAND(TARGET ${PROGNAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} ARGS -Dinput_file=${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/MacOS/${PROGNAME} -P ${CMAKE_MODULE_PATH}/OwnIngestOSXBundleLibraries.cmake
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
