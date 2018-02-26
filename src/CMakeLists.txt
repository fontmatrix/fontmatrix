# Fontmatrix src/CMakeLists.txt v1

set(QT_LIBRARIES Qt5::Core Qt5::Widgets Qt5::Svg Qt5::Sql Qt5::Xml Qt5::PrintSupport Qt5::WebKitWidgets)

set(fontmatrix_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/aboutwidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/activationwidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/activationwidgetitem.cpp
${CMAKE_CURRENT_SOURCE_DIR}/browserwidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/chartwidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/dataexport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/dataloader.cpp
${CMAKE_CURRENT_SOURCE_DIR}/dumpdialog.cpp
${CMAKE_CURRENT_SOURCE_DIR}/familywidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/filterbar.cpp
${CMAKE_CURRENT_SOURCE_DIR}/filterdata.cpp
${CMAKE_CURRENT_SOURCE_DIR}/filtersdialog.cpp
${CMAKE_CURRENT_SOURCE_DIR}/filtersdialogitem.cpp
${CMAKE_CURRENT_SOURCE_DIR}/filteritem.cpp
${CMAKE_CURRENT_SOURCE_DIR}/filtermeta.cpp
${CMAKE_CURRENT_SOURCE_DIR}/filterpanose.cpp
${CMAKE_CURRENT_SOURCE_DIR}/filtertag.cpp
${CMAKE_CURRENT_SOURCE_DIR}/floatingwidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/floatingwidgetsregister.cpp
${CMAKE_CURRENT_SOURCE_DIR}/floatingwidgettoolbar.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmactivate.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmactivationreport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmaltcontext.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmaltselector.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmbaseshaper.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmdumpinfo.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmencdata.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmfloatingpreview.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmfontcompareview.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmfontdb.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmfontextract.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmfontstrings.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmfreetypelib.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmglyphhighlight.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmglyphsview.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fminfodisplay.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmkernfeat.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmlayoptwidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmlayout.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmmatchraster.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmmissingfonthelper.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmnamelist.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmotf.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmpaths.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmplayground.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmpreviewlist.cpp
# fmprintdialog.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmrepair.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmsampletextview.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmshaper_own.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmuniblocks.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmutils.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmvariants.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmverticalbutton.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fontbook.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fontbookdialog.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fontcomparewidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fontitem.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fontmatrixshaper.cpp
${CMAKE_CURRENT_SOURCE_DIR}/glyphtosvghelper.cpp
${CMAKE_CURRENT_SOURCE_DIR}/helpbrowser.cpp
# ${CMAKE_CURRENT_SOURCE_DIR}/helpwidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/importedfontsdialog.cpp
${CMAKE_CURRENT_SOURCE_DIR}/importtags.cpp
${CMAKE_CURRENT_SOURCE_DIR}/iview.cpp
${CMAKE_CURRENT_SOURCE_DIR}/QHexView.cpp
${CMAKE_CURRENT_SOURCE_DIR}/parallelcoor.cpp
${CMAKE_CURRENT_SOURCE_DIR}/puzzleviewimp.cpp
#${CMAKE_CURRENT_SOURCE_DIR}/listdockwidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
${CMAKE_CURRENT_SOURCE_DIR}/mainviewwidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/metawidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/modeltext.cpp
${CMAKE_CURRENT_SOURCE_DIR}/openclosearrow.cpp
${CMAKE_CURRENT_SOURCE_DIR}/panosedialog.cpp
${CMAKE_CURRENT_SOURCE_DIR}/panosematch.cpp
${CMAKE_CURRENT_SOURCE_DIR}/panosemodel.cpp
${CMAKE_CURRENT_SOURCE_DIR}/panosewidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/pangoshaper.cpp
${CMAKE_CURRENT_SOURCE_DIR}/playwidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/prefspaneldialog.cpp
${CMAKE_CURRENT_SOURCE_DIR}/progressbarduo.cpp
${CMAKE_CURRENT_SOURCE_DIR}/remotedir.cpp
#${CMAKE_CURRENT_SOURCE_DIR}/savedata.cpp
${CMAKE_CURRENT_SOURCE_DIR}/sampletoolbar.cpp
${CMAKE_CURRENT_SOURCE_DIR}/samplewidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/schelptreemodel.cpp
${CMAKE_CURRENT_SOURCE_DIR}/sctextbrowser.cpp
${CMAKE_CURRENT_SOURCE_DIR}/shortcuts.cpp
${CMAKE_CURRENT_SOURCE_DIR}/systray.cpp
# ${CMAKE_CURRENT_SOURCE_DIR}/tagseteditor.cpp
${CMAKE_CURRENT_SOURCE_DIR}/tagswidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/tagswidget_listmodel.cpp
${CMAKE_CURRENT_SOURCE_DIR}/textprogression.cpp
${CMAKE_CURRENT_SOURCE_DIR}/treeitem.cpp
${CMAKE_CURRENT_SOURCE_DIR}/treemodel.cpp
${CMAKE_CURRENT_SOURCE_DIR}/tttableview.cpp
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

IF(PODOFO)
	SET(PODOFO_SRCS
	fmpdffontextractor.cpp
	)
ENDIF(PODOFO)

SET(SHAPERS_SRCS
${HARFBUZZ_SRCS}
${ICU_SRCS}
${M17N_SRCS}
)

SET(SHAPERS_LIBRARIES
${ICU_LIBRARIES}
${M17N_LIBRARIES}
)

IF(WANT_PYTHONQT)
	SET(PYTHONQT_SCRIPTER_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/fmpython_w.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/fmscriptconsole.cpp
	)
	SET(PYTHONQT_SCRIPTER_MOC_HDRS
	fmpython_w.h
	fmscriptconsole.h
	)
	SET(PYTHONQT_SCRIPTER_UIS
	${CMAKE_CURRENT_SOURCE_DIR}/scriptconsole.ui
	)
	SET(PYTHONQT_LIB PythonQt)
ENDIF(WANT_PYTHONQT)


SET(fontmatrix_MOC_HDRS
aboutwidget.h
browserwidget.h
activationwidget.h
activationwidgetitem.h
chartwidget.h
dataexport.h
dumpdialog.h
familywidget.h
filterbar.h
filterdata.h
filtersdialog.h
filtersdialogitem.h
filteritem.h
floatingwidget.h
floatingwidgettoolbar.h
fmactivate.h
fmaltcontext.h
fmaltselector.h
fmfloatingpreview.h
fmfontcompareview.h
fmfontdb.h
fmfontextract.h
fmfontstrings.h
fmfreetypelib.h
fmglyphhighlight.h
fmglyphsview.h
fmlayoptwidget.h
fmlayout.h
fmmatchraster.h
fmnamelist.h
fmplayground.h
fmpreviewlist.h
fmrepair.h
fmsampletextview.h
fontbookdialog.h
fontcomparewidget.h
fontitem.h
helpbrowser.h
# helpwidget.h
importtags.h
iview.h
QHexView.h
#listdockwidget.h
mainviewwidget.h
metawidget.h
modeltext.h
openclosearrow.h
panosedialog.h
panosewidget.h
parallelcoor.h
playwidget.h
prefspaneldialog.h
progressbarduo.h
remotedir.h
sampletoolbar.h
samplewidget.h
sctextbrowser.h
shortcuts.h
systray.h
# tagseteditor.h
tagswidget.h
tagswidget_listmodel.h
#textprogression.h
tttableview.h
typotek.h
)
QT5_WRAP_CPP(fontmatrix_MOC_SRCS 
${fontmatrix_MOC_HDRS} 
${PYTHONQT_SCRIPTER_MOC_HDRS})


SET(fontmatrix_UIS
${CMAKE_CURRENT_SOURCE_DIR}/about.ui
${CMAKE_CURRENT_SOURCE_DIR}/activationreportdialog.ui
${CMAKE_CURRENT_SOURCE_DIR}/activationwidget.ui
${CMAKE_CURRENT_SOURCE_DIR}/activationwidgetitem.ui
${CMAKE_CURRENT_SOURCE_DIR}/altselectorwidget.ui
${CMAKE_CURRENT_SOURCE_DIR}/bookexport.ui
${CMAKE_CURRENT_SOURCE_DIR}/browserwidget.ui
${CMAKE_CURRENT_SOURCE_DIR}/chartwidget.ui
${CMAKE_CURRENT_SOURCE_DIR}/comparewidget.ui
${CMAKE_CURRENT_SOURCE_DIR}/dataexport.ui
${CMAKE_CURRENT_SOURCE_DIR}/dumpdialog.ui
${CMAKE_CURRENT_SOURCE_DIR}/familywidget.ui
${CMAKE_CURRENT_SOURCE_DIR}/filterbar.ui
${CMAKE_CURRENT_SOURCE_DIR}/filtersdialog.ui
${CMAKE_CURRENT_SOURCE_DIR}/filtersdialogitem.ui
${CMAKE_CURRENT_SOURCE_DIR}/filteritem.ui
${CMAKE_CURRENT_SOURCE_DIR}/floatingwidgettoolbar.ui
${CMAKE_CURRENT_SOURCE_DIR}/fontextractordialog.ui
${CMAKE_CURRENT_SOURCE_DIR}/importedfonts.ui
${CMAKE_CURRENT_SOURCE_DIR}/importtags.ui
# ${CMAKE_CURRENT_SOURCE_DIR}/help.ui
${CMAKE_CURRENT_SOURCE_DIR}/helpbrowser.ui
${CMAKE_CURRENT_SOURCE_DIR}/layoutoptions.ui
#${CMAKE_CURRENT_SOURCE_DIR}/listsdock.ui
${CMAKE_CURRENT_SOURCE_DIR}/mainview.ui
${CMAKE_CURRENT_SOURCE_DIR}/metawidget.ui
${CMAKE_CURRENT_SOURCE_DIR}/matchraster.ui
${CMAKE_CURRENT_SOURCE_DIR}/panosedialog.ui
${CMAKE_CURRENT_SOURCE_DIR}/panosewidget.ui
${CMAKE_CURRENT_SOURCE_DIR}/playwidget.ui
${CMAKE_CURRENT_SOURCE_DIR}/prefs_panel.ui
${CMAKE_CURRENT_SOURCE_DIR}/progressbarduo.ui
${CMAKE_CURRENT_SOURCE_DIR}/repair.ui
${CMAKE_CURRENT_SOURCE_DIR}/sampletoolbar.ui
${CMAKE_CURRENT_SOURCE_DIR}/samplewidget.ui
# ${CMAKE_CURRENT_SOURCE_DIR}/tagset.ui
${CMAKE_CURRENT_SOURCE_DIR}/tagswidget.ui
${CMAKE_CURRENT_SOURCE_DIR}/textprogression.ui
${CMAKE_CURRENT_SOURCE_DIR}/tttablewidget.ui
)
QT5_WRAP_UI(fontmatrix_UIS_H 
${fontmatrix_UIS}
${PYTHONQT_SCRIPTER_UIS}
)

# extra translation files
SET(FONTMATRIX_TR_EXTRA
${CMAKE_CURRENT_SOURCE_DIR}/langs/unicode/uniblocks.cxx
)


SET(fontmatrix_RCCS
application.qrc
)
QT5_ADD_RESOURCES(fontmatrix_RCC_SRCS ${fontmatrix_RCCS})

FILE( GLOB FM_GLOB_TSS ${CMAKE_CURRENT_SOURCE_DIR}/messages/fontmatrix-*.ts )
# message(STATUS "lang files: ${FM_GLOB_TSS}")
IF ( TRANSLATOR )
	MESSAGE(STATUS "We are glad to see you handle the fontmatrix-${TRANSLATOR}.ts file, thank you very much")
	ADD_CUSTOM_TARGET(translation
			COMMAND ${QT_LUPDATE_EXECUTABLE}  ${fontmatrix_SRCS} ${fontmatrix_UIS} ${FONTMATRIX_TR_EXTRA} -ts "fontmatrix-${TRANSLATOR}.ts"
			DEPENDS ${fontmatrix_SRCS} ${fontmatrix_UIS}
			WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/messages"
			)
	SET( fontmatrix_TSS 
		${FM_GLOB_TSS} 
		"messages/fontmatrix-${TRANSLATOR}.ts" )
ELSE ( TRANSLATOR )
	SET( fontmatrix_TSS ${FM_GLOB_TSS} )
ENDIF( TRANSLATOR )





qt5_add_translation( fontmatrix_QMS ${fontmatrix_TSS} )
ADD_CUSTOM_TARGET ( translation_release ALL DEPENDS ${fontmatrix_QMS} )

# ADD_SUBDIRECTORY(messages)
ADD_SUBDIRECTORY(scripts)

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
${PYTHON_INCLUDE_PATH}
${PYTHONQT_INCLUDE_DIRS}
${LIBPODOFO_INCLUDE_DIR}
)


IF(UNIX AND NOT APPLE)
	add_executable(fontmatrix
	${fontmatrix_SRCS}
	${SHAPERS_SRCS}
	${PYTHONQT_SCRIPTER_SRCS}
	${fontmatrix_MOC_SRCS}
	${fontmatrix_UIS_H}
	${fontmatrix_RCC_SRCS}
	${PODOFO_SRCS}
	)

	target_link_libraries(fontmatrix harfbuzz hyphenate 
	${PYTHONQT_LIB}
	${QT_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${FONTCONFIG_LIBRARIES}
	${SHAPERS_LIBRARIES}
	${PYTHON_LIBRARIES}
	${LIBPODOFO_LIBRARY}
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
	${PYTHONQT_SCRIPTER_SRCS}
	${fontmatrix_MOC_SRCS}
	${fontmatrix_UIS_H}
	${fontmatrix_RCC_SRCS}
	${PODOFO_SRCS}
	)

	target_link_libraries(fontmatrix harfbuzz hyphenate
	${PYTHONQT_LIB}
	${QT_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${SHAPERS_LIBRARIES}
	${PYTHON_LIBRARIES}
	${LIBPODOFO_LIBRARY}
	"-framework ApplicationServices"
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
	ADD_CUSTOM_COMMAND( TARGET translation_release POST_BUILD
        COMMAND mkdir ARGS -p ${BUNDLE_LOCALES_DIR}
        COMMAND cp ARGS ${fontmatrix_QMS} ${BUNDLE_LOCALES_DIR}
	)
# bundles libs
#	ADD_CUSTOM_COMMAND(TARGET ${PROGNAME} POST_BUILD
#	COMMAND ${CMAKE_COMMAND} ARGS -Dinput_file=${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/MacOS/${PROGNAME} -P ${CMAKE_MODULE_PATH}/OwnIngestOSXBundleLibraries.cmake
#	)

INSTALL(TARGETS ${PROGNAME}
    BUNDLE DESTINATION . COMPONENT Runtime
    )

SET(APPLE_LIBS
	${PYTHONQT_LIB}
	${QT_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${SHAPERS_LIBRARIES}
	${PYTHON_LIBRARIES}
	${LIBPODOFO_LIBRARY}
)
SET(APPLE_DIRS "${QT_PLUGINS_DIR}/sqldrivers")
SET(APPLE_APPS "\${CMAKE_INSTALL_PREFIX}/fontmatrix.app")
INSTALL(CODE "
    include(BundleUtilities)
    fixup_bundle(\"${APPLE_APPS}\" \"${APPLE_LIBS}\" \"${APPLE_DIRS}\")
    " COMPONENT Runtime)

ENDIF(APPLE)

IF(WIN32)
	add_executable(fontmatrix WIN32
	${fontmatrix_SRCS}
	${SHAPERS_SRCS}
	${PYTHONQT_SCRIPTER_SRCS}
	${fontmatrix_MOC_SRCS}
	${fontmatrix_UIS_H}
	${fontmatrix_RCC_SRCS}
	${PODOFO_SRCS}
	)

	target_link_libraries(fontmatrix harfbuzz hyphenate
	${PYTHONQT_LIB}
	${QT_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${SHAPERS_LIBRARIES}
	${PYTHON_LIBRARIES}
	${LIBPODOFO_LIBRARY}
	)

	INSTALL(TARGETS fontmatrix
	RUNTIME DESTINATION ${FONTMATRIX_BIN}
	)
	INSTALL ( FILES ${fontmatrix_QMS} DESTINATION ${FONTMATRIX_QMDIR} )
ENDIF(WIN32)
