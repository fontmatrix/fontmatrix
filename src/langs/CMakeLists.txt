SET(dodict_SOURCES
dodict.cpp
)

ADD_EXECUTABLE(dodict ${dodict_SOURCES})
target_link_libraries(dodict ${QT_LIBRARIES})