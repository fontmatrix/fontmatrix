SET(harfbuzz_SOURCES
harfbuzz-arabic.c
harfbuzz.c
harfbuzz-gsub.c
harfbuzz-hebrew.c
harfbuzz-myanmar.c
harfbuzz-stream.c
harfbuzz-buffer.c
harfbuzz-dump.c
harfbuzz-gdef.c
harfbuzz-gpos.c
harfbuzz-hangul.c
harfbuzz-khmer.c
harfbuzz-open.c
harfbuzz-tibetan.c
harfbuzz-indic.cpp 
harfbuzz-shaper-all.cpp
harfbuzz-shaper.cpp
harfbuzz-thai.cpp
harfbuzz-impl.c
)


ADD_LIBRARY(harfbuzz
STATIC ${harfbuzz_SOURCES})


# MESSAGE(STATUS "Adding ${harfbuzz}" )

