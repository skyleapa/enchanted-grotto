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

# fixup_bundle() uses install_name_tool, which invalidates any existing code signatures.
# Re-sign the bundle so macOS doesn't kill the process with "Code Signature Invalid".
if(NOT DEFINED SIGN_IDENTITY)
  set(SIGN_IDENTITY "-") # ad-hoc signing for local testing
endif()

message(STATUS "Codesigning bundle: ${APP} (identity: ${SIGN_IDENTITY})")
execute_process(
  COMMAND codesign --force --deep --sign ${SIGN_IDENTITY} --timestamp=none "${APP}"
  RESULT_VARIABLE _codesign_res
  OUTPUT_VARIABLE _codesign_out
  ERROR_VARIABLE _codesign_err
)
if(NOT _codesign_res EQUAL 0)
  message(FATAL_ERROR "codesign failed (${_codesign_res}):\n${_codesign_out}\n${_codesign_err}")
endif()
