INCLUDE_DIRECTORIES(
${CMAKE_SOURCE_DIR}/src
)

SET(HYPHENATE_SRCS
hnjalloc.c
hyphen.c
fmhyphenator.cpp
)

ADD_LIBRARY(hyphenate
STATIC ${HYPHENATE_SRCS})
