# Fontmatrix ./CMakeLists.txt v1

project(fontmatrix) # the name of your project
cmake_minimum_required(VERSION 2.4.0)
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

# A lot from Scribus
#<< Qt 
SET (QT_MT_REQUIRED TRUE)
SET (QT_MIN_VERSION "4.3.0")
SET (QT_USE_QTXML TRUE )
FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE( ${QT_USE_FILE} )
IF (QT_FOUND)
  MESSAGE("Qt4 Found OK")
#Add our Qt definitions
#   ADD_DEFINITIONS(
#     ${QT_DEFINITIONS}
#     -DQT_CORE_LIB
#     -DQT_GUI_LIB
#     -DQT_THREAD_LIB
#     -DQT_XML_LIB
#   )
ELSE(QT_FOUND)
  MESSAGE(FATAL_ERROR "No Qt4 found")
ENDIF(QT_FOUND)
#>> Qt

#<< FreeType2
SET(FREETYPE_DIR ${CMAKE_MODULE_PATH})
FIND_PACKAGE(FREETYPE REQUIRED)
IF (FREETYPE_FOUND)
  MESSAGE("FreeType2 Library Found OK")
  INCLUDE(CheckLibraryExists)
  CHECK_LIBRARY_EXISTS(libfreetype.so FT_Get_First_Char "" HAVE_FREETYPE_FIRST)
  CHECK_LIBRARY_EXISTS(libfreetype.so FT_Get_Next_Char "" HAVE_FREETYPE_NEXT)
ENDIF(FREETYPE_FOUND)
#FIXME: if cflags empty add, otherwise replace as now
SET(CFLAGS "${LIBFREETYPE_CFLAGS}")
#>> FreeType2

# tell cmake to process CMakeLists.txt in that subdirectory
add_subdirectory(src)

# Installation

SET(DESKTOPDIR "share/applications/")
ADD_DEFINITIONS("-DDESKTOPDIR=\\\"${CMAKE_INSTALL_PREFIX}/${DESKTOPDIR}\\\"")

INSTALL(FILES
  fontmatrix.desktop
  DESTINATION ${DESKTOPDIR}
)