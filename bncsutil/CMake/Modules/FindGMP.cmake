# Try to find the GMP librairies
#  GMP_FOUND - system has GMP lib
#  GMP_INCLUDE_DIR - the GMP include directory
#  GMP_LIBRARIES - Libraries needed to use GMP

if (GMP_INCLUDE_DIR AND GMP_LIBRARIES)
set(GMP_FIND_QUIETLY TRUE)
endif (GMP_INCLUDE_DIR AND GMP_LIBRARIES)
	
find_path(GMP_INCLUDE_DIR NAMES gmp.h PATHS ${CMAKE_SOURCE_DIR}/depends/include)
find_library(GMP_LIBRARIES NAMES gmp libgmp libgmp.so.10 PATHS ${CMAKE_SOURCE_DIR}/depends/lib )

MESSAGE(STATUS "GMP libs: " ${GMP_LIBRARIES} " " ${GMPXX_LIBRARIES} )
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GMP DEFAULT_MSG GMP_INCLUDE_DIR GMP_LIBRARIES)
mark_as_advanced(GMP_INCLUDE_DIR GMP_LIBRARIES)
