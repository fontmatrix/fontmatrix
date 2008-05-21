# Finds m17n Library
#
#  M17N_FOUND
#  M17N_INCLUDE_DIRS
#  M17N_LIBRARIES

FIND_PATH(
M17N_INCLUDE_DIR 
NAMES m17n.h
DOC "Include directory for the M17N library"
)

# Look for the library.
find_library(
  M17N_LIBRARY
  NAMES m17n-core
  DOC "Library to link against core M17N"
)

find_library(
  M17N_FLT_LIBRARY
  NAMES m17n-flt
  DOC "Library to link against Layout part of M17N"
)

# message(STATUS "M17N INC "  ${M17N_INCLUDE_DIR})
# message(STATUS "M17N LIB "  ${M17N_LIBRARY} )

# Copy the results to the output variables.
if(M17N_INCLUDE_DIR AND M17N_LIBRARY AND M17N_FLT_LIBRARY)
  set(M17N_FOUND 1)
  set(M17N_LIBRARIES ${M17N_LIBRARY} ${M17N_FLT_LIBRARY})
  set(M17N_INCLUDE_DIRS ${M17N_INCLUDE_DIR})
endif(M17N_INCLUDE_DIR AND M17N_LIBRARY AND M17N_FLT_LIBRARY)
