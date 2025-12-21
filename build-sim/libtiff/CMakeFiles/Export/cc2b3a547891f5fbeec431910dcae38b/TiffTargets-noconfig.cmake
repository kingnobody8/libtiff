#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "TIFF::tiff" for configuration ""
set_property(TARGET TIFF::tiff APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(TIFF::tiff PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/tiff.framework/tiff"
  IMPORTED_SONAME_NOCONFIG "@rpath/tiff.framework/tiff"
  )

list(APPEND _cmake_import_check_targets TIFF::tiff )
list(APPEND _cmake_import_check_files_for_TIFF::tiff "${_IMPORT_PREFIX}/lib/tiff.framework/tiff" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
