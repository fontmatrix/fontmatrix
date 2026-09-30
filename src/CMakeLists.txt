# Fontmatrix src/CMakeLists.txt v1

set(QT_LIBRARIES Qt6::Core Qt6::Widgets Qt6::Svg Qt6::SvgWidgets Qt6::Sql Qt6::Xml Qt6::PrintSupport Qt6::Network Qt6::DBus)

set(fontmatrix_SRCS
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
${CMAKE_CURRENT_SOURCE_DIR}/filterduplicate.cpp
${CMAKE_CURRENT_SOURCE_DIR}/filterkind.cpp
${CMAKE_CURRENT_SOURCE_DIR}/filterlang.cpp
${CMAKE_CURRENT_SOURCE_DIR}/filterlicense.cpp
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
${CMAKE_CURRENT_SOURCE_DIR}/fmbidi.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmdumpinfo.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmduplicates.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmduplicatesdialog.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmencdata.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmfloatingpreview.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmfontcompareview.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmfontdb.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmfontkinds.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmfontextract.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmhandbook.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmhyphenator.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmlangcoverage.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmlicense.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmshaper.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmsvgglyphs.cpp
${CMAKE_CURRENT_SOURCE_DIR}/harfbuzzqtshaper.cpp
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
${CMAKE_CURRENT_SOURCE_DIR}/fmcolorglyphitem.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmcolorpainter.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmconfig.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmpaths.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmportal.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmplayground.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmpreviewlist.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmrepair.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmsampletextview.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmshaper_own.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmuniblocks.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmutils.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmvariants.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmvariationswidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fmverticalbutton.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fontbook.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fontbookdialog.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fontcomparewidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fontitem.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fontmatrixshaper.cpp
${CMAKE_CURRENT_SOURCE_DIR}/glyphtosvghelper.cpp
${CMAKE_CURRENT_SOURCE_DIR}/importedfontsdialog.cpp
${CMAKE_CURRENT_SOURCE_DIR}/importtags.cpp
${CMAKE_CURRENT_SOURCE_DIR}/iview.cpp
${CMAKE_CURRENT_SOURCE_DIR}/QHexView.cpp
${CMAKE_CURRENT_SOURCE_DIR}/parallelcoor.cpp
${CMAKE_CURRENT_SOURCE_DIR}/puzzleviewimp.cpp
${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
${CMAKE_CURRENT_SOURCE_DIR}/mainviewwidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/metawidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/modeltext.cpp
${CMAKE_CURRENT_SOURCE_DIR}/openclosearrow.cpp
${CMAKE_CURRENT_SOURCE_DIR}/panosedialog.cpp
${CMAKE_CURRENT_SOURCE_DIR}/panosematch.cpp
${CMAKE_CURRENT_SOURCE_DIR}/panosemodel.cpp
${CMAKE_CURRENT_SOURCE_DIR}/panosewidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/playwidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/prefspaneldialog.cpp
${CMAKE_CURRENT_SOURCE_DIR}/progressbarduo.cpp
${CMAKE_CURRENT_SOURCE_DIR}/remotedir.cpp
${CMAKE_CURRENT_SOURCE_DIR}/sampletoolbar.cpp
${CMAKE_CURRENT_SOURCE_DIR}/samplewidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/shortcuts.cpp
${CMAKE_CURRENT_SOURCE_DIR}/systray.cpp
${CMAKE_CURRENT_SOURCE_DIR}/tagswidget.cpp
${CMAKE_CURRENT_SOURCE_DIR}/tagswidget_listmodel.cpp
${CMAKE_CURRENT_SOURCE_DIR}/textprogression.cpp
${CMAKE_CURRENT_SOURCE_DIR}/tttableview.cpp
${CMAKE_CURRENT_SOURCE_DIR}/typotek.cpp
)

# Debug output goes through one category, which is off unless asked for:
#   QT_LOGGING_RULES="fontmatrix.debug=true" fontmatrix
ecm_qt_declare_logging_category(fontmatrix_SRCS
	HEADER fontmatrix_debug.h
	IDENTIFIER FONTMATRIX_LOG
	CATEGORY_NAME fontmatrix
	DEFAULT_SEVERITY Info
	DESCRIPTION "Fontmatrix"
	EXPORT FONTMATRIX
)
ecm_qt_install_logging_categories(
	EXPORT FONTMATRIX
	FILE fontmatrix.categories
	DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)


IF(PODOFO)
	SET(PODOFO_SRCS
	fmpdffontextractor.cpp
	)
	# PoDoFo API uses C++ exceptions; re-enable them for this file only.
	# KDE_SOURCE_FILES_ENABLE_EXCEPTIONS is only available when KDECompilerSettings
	# is loaded (Linux). On Windows/MSVC exceptions are enabled by default.
	if(COMMAND KDE_SOURCE_FILES_ENABLE_EXCEPTIONS)
		KDE_SOURCE_FILES_ENABLE_EXCEPTIONS(${CMAKE_CURRENT_SOURCE_DIR}/fmpdffontextractor.cpp)
	endif()
	IF(TARGET podofo::podofo)
		# podofo 1.0.x CMake config (namespaced target)
		SET(PODOFO_LINK_LIBS podofo::podofo)
	ELSEIF(TARGET podofo_shared)
		# podofo 0.10.x system package (old-style non-namespaced target)
		SET(PODOFO_LINK_LIBS podofo_shared)
	ELSE()
		SET(PODOFO_LINK_LIBS ${LIBPODOFO_LIBRARY})
	ENDIF()
ENDIF(PODOFO)



# AUTOMOC finds the Q_OBJECT headers itself; each source includes its moc_<name>.cpp,
# so the meta-object code is compiled with the class and not in one large extra unit.
# A build tree from before that still holds the moc_<name>.cpp files qt_wrap_cpp wrote
# here, and this directory comes first on the include path: the sources would pick up
# those instead of the current ones.
file(GLOB fontmatrix_STALE_MOC "${CMAKE_CURRENT_BINARY_DIR}/moc_*.cpp")
if(fontmatrix_STALE_MOC)
    file(REMOVE ${fontmatrix_STALE_MOC})
endif()


SET(fontmatrix_UIS
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
${CMAKE_CURRENT_SOURCE_DIR}/layoutoptions.ui
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
ki18n_wrap_ui(fontmatrix_UIS_H
${fontmatrix_UIS}
)

# extra translation files
SET(FONTMATRIX_TR_EXTRA
${CMAKE_CURRENT_SOURCE_DIR}/langs/unicode/uniblocks.cxx
)


SET(fontmatrix_RCCS
application.qrc
)
qt_add_resources(fontmatrix_RCC_SRCS ${fontmatrix_RCCS})

# Translation pipeline lives in po/ at the project root and is wired up via
# ki18n_install(po) in the top-level CMakeLists.txt. Source strings flow
# through KLocalizedString (i18n() / i18nc() / i18np()).

include_directories(
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_BINARY_DIR}
${QT_INCLUDE_DIR}
${QT_QT_INCLUDE_DIR}
${FREETYPE_INCLUDE_DIRS}
)
# only when the module found it: "LIBPODOFO_INCLUDE_DIR-NOTFOUND" is not a directory
if(PODOFO AND LIBPODOFO_INCLUDE_DIR)
	include_directories(${LIBPODOFO_INCLUDE_DIR})
endif()


IF(UNIX AND NOT APPLE)
	add_executable(fontmatrix
	${fontmatrix_SRCS}
	${fontmatrix_UIS_H}
	${fontmatrix_RCC_SRCS}
	${PODOFO_SRCS}
	)
	if(FONTMATRIX_HAVE_KAUTH)
		target_link_libraries(fontmatrix KF6::AuthCore)
		add_subdirectory(helper)
	endif()

	target_link_libraries(fontmatrix HarfBuzz::HarfBuzz Hyphen::Hyphen
	${QT_LIBRARIES}
	${FREETYPE_LIBRARIES}
	ZLIB::ZLIB
	$<TARGET_NAME_IF_EXISTS:Fontconfig::Fontconfig>
	${PODOFO_LINK_LIBS}
	KF6::CoreAddons
	KF6::I18n
	KF6::ConfigCore
	KF6::XmlGui
	KF6::ConfigWidgets
	KF6::WidgetsAddons
	KF6::StatusNotifierItem
	KF6::DBusAddons
	KF6::Crash
	KF6::IconThemes
	)

	INSTALL(TARGETS fontmatrix
	RUNTIME DESTINATION ${FONTMATRIX_BIN}
	)

	# Dev launcher: prepend ${CMAKE_BINARY_DIR}/data to XDG_DATA_DIRS so the
	# handbook (mirrored by doc/fontmatrix/CMakeLists.txt) is found by
	# khelpcenter when the binary is run from the build tree.
	file(GENERATE
	    OUTPUT  "${CMAKE_BINARY_DIR}/bin/fontmatrix-dev.sh"
	    CONTENT "#!/bin/sh
# Auto-generated dev launcher — runs the build-tree binary with XDG_DATA_DIRS
# pointed at ${CMAKE_BINARY_DIR}/data so F1 finds the handbook without install.
export XDG_DATA_DIRS=\"${CMAKE_BINARY_DIR}/data:\${XDG_DATA_DIRS:-/usr/local/share:/usr/share}\"
exec \"${CMAKE_BINARY_DIR}/bin/fontmatrix\" \"\$@\"
"
	    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
	                     GROUP_READ GROUP_EXECUTE
	                     WORLD_READ WORLD_EXECUTE
	)

	# Dev-tree mirror for the app icons. Mirrors src/icons/<size>-<context>-<name>.<ext>
	# into ${CMAKE_BINARY_DIR}/data/icons/hicolor/<size>/<context>/<name>.<ext> so
	# QIcon::fromTheme("fontmatrix") resolves when running from build/bin via
	# the dev launcher (which prepends ${CMAKE_BINARY_DIR}/data to XDG_DATA_DIRS).
	# Spec format: "<subdir>|<source-filename>|<install-filename>"
	set(_icon_dev_root "${CMAKE_BINARY_DIR}/data/icons/hicolor")
	set(_icon_dev_specs
		"16/apps|16-apps-fontmatrix.png|fontmatrix.png"
		"22/apps|22-apps-fontmatrix.png|fontmatrix.png"
		"32/apps|32-apps-fontmatrix.png|fontmatrix.png"
		"48/apps|48-apps-fontmatrix.png|fontmatrix.png"
		"64/apps|64-apps-fontmatrix.png|fontmatrix.png"
		"128/apps|128-apps-fontmatrix.png|fontmatrix.png"
		"scalable/apps|sc-apps-fontmatrix.svg|fontmatrix.svg"
		"22/status|22-status-fontmatrix-tray.png|fontmatrix-tray.png"
		"scalable/status|sc-status-fontmatrix-tray.svg|fontmatrix-tray.svg"
	)
	set(_icon_dev_outputs)
	foreach(_spec IN LISTS _icon_dev_specs)
		string(REPLACE "|" ";" _parts "${_spec}")
		list(GET _parts 0 _subdir)
		list(GET _parts 1 _src)
		list(GET _parts 2 _dst)
		set(_src_path "${CMAKE_SOURCE_DIR}/src/icons/${_src}")
		set(_dst_path "${_icon_dev_root}/${_subdir}/${_dst}")
		add_custom_command(
			OUTPUT  "${_dst_path}"
			COMMAND ${CMAKE_COMMAND} -E make_directory "${_icon_dev_root}/${_subdir}"
			COMMAND ${CMAKE_COMMAND} -E copy "${_src_path}" "${_dst_path}"
			DEPENDS "${_src_path}"
			COMMENT "Mirroring icon ${_src} → hicolor/${_subdir}/${_dst}"
			VERBATIM
		)
		list(APPEND _icon_dev_outputs "${_dst_path}")
	endforeach()
	add_custom_target(fontmatrix-icons-devtree ALL DEPENDS ${_icon_dev_outputs})

	# Dev-tree mirror for the .mo translation catalogue. ki18n_install(po)
	# generates ${CMAKE_BINARY_DIR}/locale/<lang>/LC_MESSAGES/fontmatrix.mo
	# at build time, but KLocalizedString resolves catalogues via
	# QStandardPaths::GenericDataLocation/locale — i.e. <XDG_DATA_DIR>/locale.
	# The dev launcher prepends ${CMAKE_BINARY_DIR}/data to XDG_DATA_DIRS, so
	# we expose the build-tree locale dir there via a symlink. This is what
	# makes the "Switch Application Language" dialog (KHelpMenu) discover
	# every language without `make install`.
	add_custom_command(
		OUTPUT  "${CMAKE_BINARY_DIR}/data/locale"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/data"
		COMMAND ${CMAKE_COMMAND} -E create_symlink
		        "${CMAKE_BINARY_DIR}/locale"
		        "${CMAKE_BINARY_DIR}/data/locale"
		COMMENT "Linking ${CMAKE_BINARY_DIR}/data/locale → ${CMAKE_BINARY_DIR}/locale"
		VERBATIM
	)
	add_custom_target(fontmatrix-locale-devtree ALL
		DEPENDS "${CMAKE_BINARY_DIR}/data/locale" pofiles)
ENDIF(UNIX AND NOT APPLE)

IF(APPLE)
	# The icon is a source of the bundle, which puts it in Contents/Resources.
	set(fontmatrix_ICNS "${CMAKE_SOURCE_DIR}/src/graphic-resources/fontmatrix.icns")
	set_source_files_properties(${fontmatrix_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

	add_executable(fontmatrix MACOSX_BUNDLE
	${fontmatrix_SRCS}
	${fontmatrix_UIS_H}
	${fontmatrix_RCC_SRCS}
	${PODOFO_SRCS}
	${fontmatrix_ICNS}
	)

	target_link_libraries(fontmatrix HarfBuzz::HarfBuzz Hyphen::Hyphen
	${QT_LIBRARIES}
	${FREETYPE_LIBRARIES}
	ZLIB::ZLIB
	${PODOFO_LINK_LIBS}
	"-framework ApplicationServices"
	KF6::CoreAddons
	KF6::I18n
	KF6::ConfigCore
	KF6::XmlGui
	KF6::ConfigWidgets
	KF6::WidgetsAddons
	KF6::StatusNotifierItem
	KF6::DBusAddons
	KF6::Crash
	KF6::IconThemes
	)

	set_target_properties(fontmatrix PROPERTIES
	MACOSX_BUNDLE_BUNDLE_NAME "Fontmatrix"
	MACOSX_BUNDLE_ICON_FILE "fontmatrix.icns"
	MACOSX_BUNDLE_GUI_IDENTIFIER "com.github.fontmatrix.Fontmatrix"
	MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}"
	MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}"
	MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}"
	MACOSX_BUNDLE_COPYRIGHT "© 2007–2026 Fontmatrix contributors"
	)

	# Only the bundle itself is installed here: the data goes through the
	# KDE_INSTALL_* variables like on every platform, under share/, and Craft's
	# Mac packager moves share/ into Contents/Resources and bundles the
	# libraries into Contents/Frameworks. Craft points KDE_INSTALL_BUNDLEDIR
	# at <CraftRoot>/Applications/KDE.
	INSTALL(TARGETS fontmatrix
	BUNDLE DESTINATION "${KDE_INSTALL_BUNDLEDIR}"
	)
ENDIF(APPLE)

IF(WIN32)
	add_executable(fontmatrix WIN32
	${fontmatrix_SRCS}
	${fontmatrix_UIS_H}
	${fontmatrix_RCC_SRCS}
	${PODOFO_SRCS}
	fontmatrix.rc
	)

	target_link_libraries(fontmatrix HarfBuzz::HarfBuzz Hyphen::Hyphen
	${QT_LIBRARIES}
	${FREETYPE_LIBRARIES}
	ZLIB::ZLIB
	${PODOFO_LINK_LIBS}
	KF6::CoreAddons
	KF6::I18n
	KF6::ConfigCore
	KF6::XmlGui
	KF6::ConfigWidgets
	KF6::WidgetsAddons
	KF6::StatusNotifierItem
	KF6::DBusAddons
	KF6::Crash
	KF6::IconThemes
	# font activation: the per-user font registry key and AddFontResource()
	advapi32
	gdi32
	)

	INSTALL(TARGETS fontmatrix
	RUNTIME DESTINATION ${FONTMATRIX_BIN}
	)
ENDIF(WIN32)

# QT_NO_CAST_FROM_ASCII, one file at a time. The project removes the definition
# globally (see the top-level CMakeLists.txt), because a literal that reaches the
# user has to become i18n() and not QStringLiteral(), which is a decision per
# string. The files below hold no text for the user and are done: the definition
# keeps an implicit char* to QString conversion from coming back into them. To
# add a file, convert it, list it here, build.
set_source_files_properties(
	dataloader.cpp
	dumpdialog.cpp
	familywidget.cpp
	filterdata.cpp
	filteritem.cpp
	filtermeta.cpp
	filterduplicate.cpp
	filterkind.cpp
	filterlang.cpp
	filterlicense.cpp
	filterpanose.cpp
	filtertag.cpp
	floatingwidget.cpp
	fmaltselector.cpp
	fmbaseshaper.cpp
	fmbidi.cpp
	fmcolorglyphitem.cpp
	fmcolorpainter.cpp
	fmconfig.cpp
	fmdumpinfo.cpp
	fmduplicates.cpp
	fmduplicatesdialog.cpp
	fmencdata.cpp
	fmfontcompareview.cpp
	fmfontdb.cpp
	fmfontkinds.cpp
	fmglyphsview.cpp
	fmhandbook.cpp
	fmkernfeat.cpp
	fmlangcoverage.cpp
	fmlicense.cpp
	fmlayoptwidget.cpp
	fmnamelist.cpp
	fmotf.cpp
	fmpaths.cpp
	fmpdffontextractor.cpp
	fmportal.cpp
	fmplayground.cpp
	fmshaper.cpp
	fmsvgglyphs.cpp
	fmshaper_own.cpp
	fmuniblocks.cpp
	fmutils.cpp
	fmvariants.cpp
	fmvariationswidget.cpp
	glyphtosvghelper.cpp
	modeltext.cpp
	openclosearrow.cpp
	panosedialog.cpp
	panosematch.cpp
	panosemodel.cpp
	panosewidget.cpp
	parallelcoor.cpp
	QHexView.cpp
	shortcuts.cpp
	PROPERTIES COMPILE_DEFINITIONS "QT_NO_CAST_FROM_ASCII")

# The project's own sources build without a warning. Off by default: a
# packager's newer compiler must not turn a new diagnostic into a failed build.
option(FONTMATRIX_WERROR "Treat compiler warnings as errors in Fontmatrix's own sources (for CI)" OFF)
if(FONTMATRIX_WERROR)
	target_compile_options(fontmatrix PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>,/WX,-Werror>)
endif()

# typotek calls setXMLFile("fontmatrixui.rc") + createGUI(); without this file
# KXmlGui builds no menu bar and no toolbar. Installed for every platform, not
# per-branch.
INSTALL(FILES fontmatrixui.rc DESTINATION "${KDE_INSTALL_KXMLGUIDIR}/fontmatrix")
