# Install script for directory: /Users/maximilian/CLionProjects/ghostpp/bncsutil

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

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/Users/maximilian/CLionProjects/ghostpp/bncsutil/build/libbncsutil.1.4.1.dylib"
    "/Users/maximilian/CLionProjects/ghostpp/bncsutil/build/libbncsutil.1.dylib"
    "/Users/maximilian/CLionProjects/ghostpp/bncsutil/build/libbncsutil.dylib"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbncsutil.1.4.1.dylib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbncsutil.1.dylib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbncsutil.dylib"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      execute_process(COMMAND "/usr/bin/install_name_tool"
        -id "libbncsutil.1.dylib"
        "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/bncsutil" TYPE FILE FILES
    "/Users/maximilian/CLionProjects/ghostpp/bncsutil/src/bncsutil/bncsutil.h"
    "/Users/maximilian/CLionProjects/ghostpp/bncsutil/src/bncsutil/bsha1.h"
    "/Users/maximilian/CLionProjects/ghostpp/bncsutil/src/bncsutil/buffer.h"
    "/Users/maximilian/CLionProjects/ghostpp/bncsutil/src/bncsutil/cdkeydecoder.h"
    "/Users/maximilian/CLionProjects/ghostpp/bncsutil/src/bncsutil/checkrevision.h"
    "/Users/maximilian/CLionProjects/ghostpp/bncsutil/src/bncsutil/decodekey.h"
    "/Users/maximilian/CLionProjects/ghostpp/bncsutil/src/bncsutil/file.h"
    "/Users/maximilian/CLionProjects/ghostpp/bncsutil/src/bncsutil/keytables.h"
    "/Users/maximilian/CLionProjects/ghostpp/bncsutil/src/bncsutil/libinfo.h"
    "/Users/maximilian/CLionProjects/ghostpp/bncsutil/src/bncsutil/ms_stdint.h"
    "/Users/maximilian/CLionProjects/ghostpp/bncsutil/src/bncsutil/mutil.h"
    "/Users/maximilian/CLionProjects/ghostpp/bncsutil/src/bncsutil/mutil_types.h"
    "/Users/maximilian/CLionProjects/ghostpp/bncsutil/src/bncsutil/nls.h"
    "/Users/maximilian/CLionProjects/ghostpp/bncsutil/src/bncsutil/oldauth.h"
    "/Users/maximilian/CLionProjects/ghostpp/bncsutil/src/bncsutil/pe.h"
    "/Users/maximilian/CLionProjects/ghostpp/bncsutil/src/bncsutil/sha1.h"
    "/Users/maximilian/CLionProjects/ghostpp/bncsutil/src/bncsutil/stack.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/Users/maximilian/CLionProjects/ghostpp/bncsutil/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
