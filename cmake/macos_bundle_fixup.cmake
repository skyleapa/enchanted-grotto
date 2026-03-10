# Usage:
#   cmake -DAPP=/path/to/MyGame.app [-DSEARCH_DIRS="...;..."] -P cmake/macos_bundle_fixup.cmake
#
# Copies non-system dylibs into Contents/Frameworks and rewrites install names so the app is portable.

if(NOT DEFINED APP)
  message(FATAL_ERROR "APP not set. Pass -DAPP=/path/to/<name>.app")
endif()

if(NOT IS_DIRECTORY "${APP}")
  message(FATAL_ERROR "APP does not exist or is not a directory: ${APP}")
endif()

include(BundleUtilities)

set(_dirs "")
if(DEFINED SEARCH_DIRS)
  # Expect a CMake-style list, e.g. "/opt/homebrew/lib;/usr/local/lib"
  set(_dirs ${SEARCH_DIRS})
endif()

message(STATUS "Fixing up bundle: ${APP}")
fixup_bundle("${APP}" "" "${_dirs}")

