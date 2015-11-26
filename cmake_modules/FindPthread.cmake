# Locate pthread library
# This module defines PTHREAD_FOUND, PTHREAD_INCLUDE_DIR and PTHREAD_LIBRARIES standard variables

set(PTHREAD_SEARCH_PATHS
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
	/usr/local
	/usr
	~/Library/Frameworks
	/Library/Frameworks
)
	
find_path(PTHREAD_INCLUDE_DIR
	NAMES pthread.h
	HINTS
	$ENV{PTHREAD_DIR}
	${PTHREAD_DIR}
	PATH_SUFFIXES inc include
	PATHS ${PTHREAD_SEARCH_PATHS}
)

find_library(PTHREAD_LIBRARY
	NAMES pthread pthreadGC2 pthreadVC2
	HINTS
	$ENV{PTHREAD_DIR}
	${PTHREAD_DIR}
	PATH_SUFFIXES lib lib64
	PATHS ${PTHREAD_SEARCH_PATHS}
)
	
if(PTHREAD_LIBRARY)
    set(PTHREAD_LIBRARIES "${PTHREAD_LIBRARY}")		# Could add "general" keyword, but it is optional
endif()
	
# handle the QUIETLY and REQUIRED arguments and set PTHREAD_FOUND to TRUE if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(pthread DEFAULT_MSG PTHREAD_LIBRARIES PTHREAD_INCLUDE_DIR)
