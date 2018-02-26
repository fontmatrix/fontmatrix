include_directories(
${CMAKE_SOURCE_DIR}/src
)

set(HYPHENATE_SRCS
hnjalloc.c
hyphen.c
fmhyphenator.cpp
)

add_library(hyphenate
STATIC ${HYPHENATE_SRCS})
target_link_libraries(hyphenate Qt5::Core)
