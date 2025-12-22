# Install script for directory: /Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/tools

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE DIRECTORY FILES "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/tools/fax2ps.app" USE_SOURCE_PERMISSIONS)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/fax2ps.app/fax2ps" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/fax2ps.app/fax2ps")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/libtiff"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/fax2ps.app/fax2ps")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE DIRECTORY FILES "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/tools/fax2tiff.app" USE_SOURCE_PERMISSIONS)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/fax2tiff.app/fax2tiff" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/fax2tiff.app/fax2tiff")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/libtiff"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/fax2tiff.app/fax2tiff")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE DIRECTORY FILES "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/tools/pal2rgb.app" USE_SOURCE_PERMISSIONS)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/pal2rgb.app/pal2rgb" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/pal2rgb.app/pal2rgb")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/libtiff"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/pal2rgb.app/pal2rgb")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE DIRECTORY FILES "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/tools/ppm2tiff.app" USE_SOURCE_PERMISSIONS)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/ppm2tiff.app/ppm2tiff" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/ppm2tiff.app/ppm2tiff")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/libtiff"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/ppm2tiff.app/ppm2tiff")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE DIRECTORY FILES "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/tools/raw2tiff.app" USE_SOURCE_PERMISSIONS)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/raw2tiff.app/raw2tiff" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/raw2tiff.app/raw2tiff")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/libtiff"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/raw2tiff.app/raw2tiff")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE DIRECTORY FILES "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/tools/tiff2bw.app" USE_SOURCE_PERMISSIONS)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiff2bw.app/tiff2bw" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiff2bw.app/tiff2bw")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/libtiff"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiff2bw.app/tiff2bw")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE DIRECTORY FILES "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/tools/tiff2pdf.app" USE_SOURCE_PERMISSIONS)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiff2pdf.app/tiff2pdf" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiff2pdf.app/tiff2pdf")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/libtiff"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiff2pdf.app/tiff2pdf")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE DIRECTORY FILES "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/tools/tiff2ps.app" USE_SOURCE_PERMISSIONS)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiff2ps.app/tiff2ps" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiff2ps.app/tiff2ps")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/libtiff"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiff2ps.app/tiff2ps")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE DIRECTORY FILES "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/tools/tiff2rgba.app" USE_SOURCE_PERMISSIONS)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiff2rgba.app/tiff2rgba" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiff2rgba.app/tiff2rgba")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/libtiff"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiff2rgba.app/tiff2rgba")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE DIRECTORY FILES "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/tools/tiffcmp.app" USE_SOURCE_PERMISSIONS)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffcmp.app/tiffcmp" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffcmp.app/tiffcmp")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/libtiff"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffcmp.app/tiffcmp")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE DIRECTORY FILES "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/tools/tiffcp.app" USE_SOURCE_PERMISSIONS)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffcp.app/tiffcp" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffcp.app/tiffcp")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/libtiff"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffcp.app/tiffcp")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE DIRECTORY FILES "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/tools/tiffcrop.app" USE_SOURCE_PERMISSIONS)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffcrop.app/tiffcrop" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffcrop.app/tiffcrop")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/libtiff"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffcrop.app/tiffcrop")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE DIRECTORY FILES "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/tools/tiffdither.app" USE_SOURCE_PERMISSIONS)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffdither.app/tiffdither" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffdither.app/tiffdither")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/libtiff"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffdither.app/tiffdither")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE DIRECTORY FILES "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/tools/tiffdump.app" USE_SOURCE_PERMISSIONS)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffdump.app/tiffdump" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffdump.app/tiffdump")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/libtiff"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffdump.app/tiffdump")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE DIRECTORY FILES "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/tools/tiffinfo.app" USE_SOURCE_PERMISSIONS)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffinfo.app/tiffinfo" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffinfo.app/tiffinfo")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/libtiff"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffinfo.app/tiffinfo")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE DIRECTORY FILES "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/tools/tiffmedian.app" USE_SOURCE_PERMISSIONS)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffmedian.app/tiffmedian" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffmedian.app/tiffmedian")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/libtiff"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffmedian.app/tiffmedian")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE DIRECTORY FILES "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/tools/tiffset.app" USE_SOURCE_PERMISSIONS)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffset.app/tiffset" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffset.app/tiffset")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/libtiff"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffset.app/tiffset")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE DIRECTORY FILES "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/tools/tiffsplit.app" USE_SOURCE_PERMISSIONS)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffsplit.app/tiffsplit" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffsplit.app/tiffsplit")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/libtiff"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tiffsplit.app/tiffsplit")
  endif()
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/alaa/Developer/Git/Contribution/iOS_Port/libtiff/build-sim/tools/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
