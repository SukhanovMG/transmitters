
#  GPERFTOOLS_FOUND              System has Gperftools libs/headers
#  GPERFTOOLS_LIBRARIES          The Gperftools libraries (tcmalloc & profiler)


find_library(GPERFTOOLS_TCMALLOC
  NAMES tcmalloc
  PATHS
      /usr/lib64
      /usr/lib
      /lib
      /lib64
)

find_library(GPERFTOOLS_PROFILER
  NAMES profiler
    PATHS
      /usr/lib64
      /usr/lib
      /lib
      /lib64
)
find_library(GPERFTOOLS_TCMALLOC_AND_PROFILER
  NAMES tcmalloc_and_profiler
   PATHS
      /usr/lib64
      /usr/lib
      /lib
      /lib64
)
set(GPERFTOOLS_LIBRARIES ${GPERFTOOLS_TCMALLOC_AND_PROFILER})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Gperftools
  DEFAULT_MSG
  GPERFTOOLS_LIBRARIES
)

mark_as_advanced(
  GPERFTOOLS_TCMALLOC
  GPERFTOOLS_PROFILER
  GPERFTOOLS_TCMALLOC_AND_PROFILER
  GPERFTOOLS_LIBRARIES
)