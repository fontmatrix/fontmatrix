# A contributed CMakeLists file (pm)
# src/CMakeLists.txt
SET(PythonQtHDRS
PythonQt.h
PythonQtStdDecorators.h
PythonQtClassInfo.h
PythonQtImporter.h
PythonQtObjectPtr.h
PythonQtSlot.h
PythonQtStdOut.h
PythonQtMisc.h
PythonQtMethodInfo.h
PythonQtImportFileInterface.h
PythonQtConversion.h
PythonQtSignalReceiver.h
PythonQtWrapper.h
PythonQtMetaObjectWrapper.h
PythonQtCppWrapperFactory.h
PythonQtVariants.h
PythonQtVariantWrapper.h
wrapper/PythonQtWrappedVariants.h
gui/PythonQtScriptingConsole.h
PythonQtSystem.h
)

SET(PythonQtMOC_HDRS
PythonQt.h
PythonQtStdDecorators.h
PythonQtSignalReceiver.h
wrapper/PythonQtWrappedVariants.h
gui/PythonQtScriptingConsole.h
)

QT4_WRAP_CPP(PythonQtMoced ${PythonQtMOC_HDRS})

SET(PythonQtSRCS
PythonQtStdDecorators.cpp
PythonQt.cpp
PythonQtClassInfo.cpp  
PythonQtImporter.cpp
PythonQtObjectPtr.cpp
PythonQtStdOut.cpp
PythonQtSlot.cpp
PythonQtMisc.cpp
PythonQtMethodInfo.cpp
PythonQtConversion.cpp
PythonQtSignalReceiver.cpp
PythonQtVariants.cpp
PythonQtVariantWrapper.cpp
PythonQtWrapper.cpp
PythonQtMetaObjectWrapper.cpp
gui/PythonQtScriptingConsole.cpp
)

include_directories(
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_BINARY_DIR}
${QT_INCLUDE_DIR}
${QT_QT_INCLUDE_DIR}
${PYTHON_INCLUDE_PATH}
)

SET(HACK_LINK util)

ADD_LIBRARY(PythonQt STATIC ${PythonQtSRCS} ${PythonQtMoced})
TARGET_LINK_LIBRARIES(PythonQt ${HACK_LINK})
