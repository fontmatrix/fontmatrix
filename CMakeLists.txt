# Fontmatrix ./CMakeLists.txt v1

PROJECT(fontmatrix) # the name of your project
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)

SET (VERSION_MAJOR "0")
SET (VERSION_MINOR "9")
SET (VERSION_PATCH "99")
SET (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
ADD_DEFINITIONS(-DFONTMATRIX_VERSION_MAJOR=${VERSION_MAJOR} -DFONTMATRIX_VERSION_MINOR=${VERSION_MINOR} -DFONTMATRIX_VERSION_PATCH=${VERSION_PATCH})
MESSAGE(STATUS "Building Fontmatrix ${VERSION}")


IF(UNIX AND NOT APPLE)
	SET (CMAKE_INSTALL_PREFIX /usr/local CACHE PATH "Installation path")
	SET (FONTMATRIX_BIN bin )
	SET (FONTMATRIX_QMDIR "share/fontmatrix/qm" )
ENDIF(UNIX AND NOT APPLE)

IF(APPLE)
# 	SET (CMAKE_INSTALL_PREFIX /Applications CACHE PATH "Installation path")
# 	SET (FONTMATRIX_BIN bin "")
	set( CMAKE_OSX_ARCHITECTURES i386 )
ENDIF(APPLE)

IF(WIN32)
	SET (CMAKE_INSTALL_PREFIX c:/fontmatrix CACHE PATH "Installation path")
	SET (FONTMATRIX_BIN "")
	SET (FONTMATRIX_QMDIR "qm" )
ENDIF(WIN32)

ADD_DEFINITIONS(-DPREFIX="${CMAKE_INSTALL_PREFIX}")
MESSAGE(STATUS "Fontmatrix will be installed under ${CMAKE_INSTALL_PREFIX}")



SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

# Very generic definition, mainly to sort out bugs reported by mrdocs
IF(APPLE)
	ADD_DEFINITIONS( -DPLATFORM_APPLE )
ENDIF(APPLE)

#<< harfbuzz (shaper)
IF(WANT_HARFBUZZ)
# Since Harfbuzz source code is included into
# FM source tree, it’s just a matter of activation
# and « cohérence »
	ADD_DEFINITIONS( -DHAVE_HARFBUZZ )
	SET(HARFBUZZSHAPER 1)
	MESSAGE(STATUS "Build Harbuzz shaper support")
ENDIF(WANT_HARFBUZZ)
#>> harfbuzz (shaper)

#<< ICU
IF(WANT_ICU)
# 	SET(ICU_DIR ${CMAKE_MODULE_PATH})
	FIND_PACKAGE(ICU)
	if(ICU_FOUND)
		MESSAGE(STATUS "ICU Found OK")
		ADD_DEFINITIONS( -DHAVE_ICU )
		SET(ICUSHAPER 1)
		MESSAGE(STATUS "Build ICU shaper support")
	ELSE(ICU_FOUND)
		MESSAGE(FATAL_ERROR "ICU Not Found ARGH")
	ENDIF(ICU_FOUND)
ENDIF(WANT_ICU)

#>> ICU

#<< m17n
IF(WANT_M17N)
# 	SET(M17N_DIR ${CMAKE_MODULE_PATH})
	FIND_PACKAGE(M17N)
	if(M17N_FOUND)
		MESSAGE(STATUS "M17N Found OK")
		ADD_DEFINITIONS( -DHAVE_M17N )
		SET(M17NSHAPER 1)
		MESSAGE(STATUS "Build m17n shaper support")
	ELSE(M17N_FOUND)
		MESSAGE(FATAL_ERROR "M17N Not Found ARGH")
	ENDIF(M17N_FOUND)
ENDIF(WANT_M17N)
#>> m17n

IF(WANT_PODOFO)
# 	SET(LIBPODOFO_DIR ${CMAKE_MODULE_PATH})
	FIND_PACKAGE(LIBPODOFO)
	IF(LIBPODOFO_FOUND)
		MESSAGE(STATUS "PoDoFo Library Found OK")
		ADD_DEFINITIONS(-DHAVE_PODOFO)
		SET(PODOFO 1)
	ENDIF(LIBPODOFO_FOUND)
ENDIF(WANT_PODOFO)


# I can’t remember what it does :|
ADD_DEFINITIONS( -DHIGH_PERF )


IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
	MESSAGE(STATUS "Debug build")
	ADD_DEFINITIONS(-DBUILD_TYPE_DEBUG)
ELSE(CMAKE_BUILD_TYPE STREQUAL "Debug")
	ADD_DEFINITIONS(-DQT_NO_DEBUG_OUTPUT)
ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

# A lot from Scribus
#<< Qt 
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5Sql REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Qt5WebKitWidgets REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
#>> Qt

SET(HARFBUZZ_DIR harfbuzz/src)
ADD_SUBDIRECTORY(${HARFBUZZ_DIR})
MESSAGE(STATUS "Adding "${CMAKE_SOURCE_DIR}/${HARFBUZZ_DIR})
SET(HARFBUZZ_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/${HARFBUZZ_DIR})

# SET(PUZZLE_DIR libpuzzle/src)
# ADD_SUBDIRECTORY(${PUZZLE_DIR})
# MESSAGE(STATUS "Adding "${CMAKE_SOURCE_DIR}/${PUZZLE_DIR})
# SET(PUZZLE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/${PUZZLE_DIR})

#<< FreeType2
# We give a try to the built-in cmake Freetype.cmake
FIND_PACKAGE(Freetype REQUIRED)
IF (FREETYPE_FOUND)
  MESSAGE(STATUS "FreeType2 Library Found OK")
ELSE (FREETYPE_FOUND)
  MESSAGE(FATAL_ERROR "No Freetype found")  
ENDIF(FREETYPE_FOUND)
#>> FreeType2

IF(WANT_PYTHONQT)
	FIND_PACKAGE(PythonLibs REQUIRED)
	if(PYTHONLIBS_FOUND)
		MESSAGE(STATUS "Python Library Found OK")
	else(PYTHONLIBS_FOUND)
		MESSAGE(FATAL_ERROR "No Python Library found")
	endif(PYTHONLIBS_FOUND)
	SET(PYTHONQT_INCLUDE_DIRS  ${CMAKE_SOURCE_DIR}/pythonqt/src)
	ADD_SUBDIRECTORY(${PYTHONQT_INCLUDE_DIRS})
	ADD_DEFINITIONS(-DHAVE_PYTHONQT)
ENDIF(WANT_PYTHONQT)

IF(UNIX AND NOT APPLE)
	SET(FONTCONFIG_DIR ${CMAKE_MODULE_PATH})
	FIND_PACKAGE(FONTCONFIG)
	IF(FONTCONFIG_FOUND)
		MESSAGE(STATUS "FontConfig Found OK")
		ADD_DEFINITIONS( -DHAVE_FONTCONFIG )
	ENDIF(FONTCONFIG_FOUND)
ENDIF(UNIX AND NOT APPLE)


SET(FM_TEMPLATES_FILES
${CMAKE_SOURCE_DIR}/src/graphic-resources/template_oneline.xml
${CMAKE_SOURCE_DIR}/src/graphic-resources/template_oneline.png
${CMAKE_SOURCE_DIR}/src/graphic-resources/template_default.xml
${CMAKE_SOURCE_DIR}/src/graphic-resources/template_default.png
)
SET(FM_STYLE_FILES
${CMAKE_SOURCE_DIR}/src/graphic-resources/html-css/info.css
${CMAKE_SOURCE_DIR}/src/graphic-resources/html-css/fontmatrix.js
${CMAKE_SOURCE_DIR}/src/graphic-resources/html-css/img/css-info-propertyname.png
${CMAKE_SOURCE_DIR}/src/graphic-resources/html-css/img/css-info-property.png
${CMAKE_SOURCE_DIR}/src/graphic-resources/html-css/img/css-shadow.png
${CMAKE_SOURCE_DIR}/src/graphic-resources/html-css/img/css-tech-info.png
)
SET(FM_WELCOME_FILES
${CMAKE_SOURCE_DIR}/src/messages/welcome_cs.html
${CMAKE_SOURCE_DIR}/src/messages/welcome_da.html
${CMAKE_SOURCE_DIR}/src/messages/welcome_de.html
${CMAKE_SOURCE_DIR}/src/messages/welcome_en.html
${CMAKE_SOURCE_DIR}/src/messages/welcome_fi.html
${CMAKE_SOURCE_DIR}/src/messages/welcome_fr.html
${CMAKE_SOURCE_DIR}/src/messages/welcome_hi.html
${CMAKE_SOURCE_DIR}/src/messages/welcome_nb.html
${CMAKE_SOURCE_DIR}/src/messages/welcome_nl.html
${CMAKE_SOURCE_DIR}/src/messages/welcome_no.html
${CMAKE_SOURCE_DIR}/src/messages/welcome_ru.html
${CMAKE_SOURCE_DIR}/src/messages/welcome_sr.html
${CMAKE_SOURCE_DIR}/src/messages/welcome_sv.html
${CMAKE_SOURCE_DIR}/src/messages/welcome_uk.html
${CMAKE_SOURCE_DIR}/src/messages/welcome_zh_CN.html
${CMAKE_SOURCE_DIR}/src/messages/welcome_zh_TW.html
)

# tell cmake to process CMakeLists.txt in that subdirectory
SET(HYPHENATE_DIR src/hyphenate)
ADD_SUBDIRECTORY(${HYPHENATE_DIR})
SET(HYPHENATE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/${HYPHENATE_DIR})



ADD_SUBDIRECTORY(src)

# dodict
# ADD_SUBDIRECTORY(src/langs)


######## Installation ##################

IF(UNIX AND NOT APPLE)
	SET(DESKTOPDIR "share/applications/")
	SET(MANDIR "share/man/")
	SET(ICONDIR "share/icons/hicolor/48x48/apps/")
	SET(RESOURCESDIR "share/fontmatrix/resources/")
	SET(HELPDIR "share/fontmatrix/help/")
	
	INSTALL(FILES
	fontmatrix.desktop
	DESTINATION ${DESKTOPDIR}
	)
	
	INSTALL(FILES
	fontmatrix.png
	DESTINATION ${ICONDIR}
	)
	
	INSTALL(FILES
	fontmatrix.1
	DESTINATION "${MANDIR}man1"
	)
	
	INSTALL(FILES
	${FM_TEMPLATES_FILES}
	${FM_STYLE_FILES}
	${FM_WELCOME_FILES}
	DESTINATION "${RESOURCESDIR}"
	)

	INSTALL(FILES
	${FM_HELP_FILES}
	DESTINATION "${HELPDIR}"
	)
ENDIF(UNIX AND NOT APPLE)

IF(APPLE)
# we do not "install" on apple’s OS
ENDIF(APPLE)

IF(WIN32)
	SET(RESOURCESDIR "share/resources/")
	SET(HELPDIR "help/")
	INSTALL(FILES
	${FM_TEMPLATES_FILES}
	${FM_STYLE_FILES}
	${FM_WELCOME_FILES}
	${FM_HELP_FILES}
	DESTINATION "${RESOURCESDIR}"
	)
	
	INSTALL(FILES
	${UNICODE_RESOURCES}
	DESTINATION "${RESOURCESDIR}/Unicode/"
	)
	
ENDIF(WIN32)

ADD_SUBDIRECTORY(help)
ADD_SUBDIRECTORY(samples)

SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})

INCLUDE(CPack)
