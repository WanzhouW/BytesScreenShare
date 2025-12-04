# Install script for directory: E:/vsc/BytesScreenShare/third_party/libdatachannel

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/shared_screen")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
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
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "E:/vsc/BytesScreenShare/build/third_party/libdatachannel/Debug/datachannel.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "E:/vsc/BytesScreenShare/build/third_party/libdatachannel/Release/datachannel.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "E:/vsc/BytesScreenShare/build/third_party/libdatachannel/MinSizeRel/datachannel.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "E:/vsc/BytesScreenShare/build/third_party/libdatachannel/RelWithDebInfo/datachannel.lib")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "E:/vsc/BytesScreenShare/build/third_party/libdatachannel/Debug/datachannel.dll")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "E:/vsc/BytesScreenShare/build/third_party/libdatachannel/Release/datachannel.dll")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "E:/vsc/BytesScreenShare/build/third_party/libdatachannel/MinSizeRel/datachannel.dll")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "E:/vsc/BytesScreenShare/build/third_party/libdatachannel/RelWithDebInfo/datachannel.dll")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/rtc" TYPE FILE FILES
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/candidate.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/channel.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/configuration.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/datachannel.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/dependencydescriptor.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/description.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/iceudpmuxlistener.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/mediahandler.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/rtcpreceivingsession.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/common.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/global.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/message.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/frameinfo.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/peerconnection.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/reliability.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/rtc.h"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/rtc.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/rtp.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/track.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/websocket.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/websocketserver.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/rtppacketizationconfig.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/rtcpsrreporter.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/rtppacketizer.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/rtpdepacketizer.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/h264rtppacketizer.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/h264rtpdepacketizer.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/nalunit.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/h265rtppacketizer.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/h265rtpdepacketizer.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/h265nalunit.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/av1rtppacketizer.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/rtcpnackresponder.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/utils.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/plihandler.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/pacinghandler.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/rembhandler.hpp"
    "E:/vsc/BytesScreenShare/third_party/libdatachannel/include/rtc/version.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/LibDataChannel/LibDataChannelTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/LibDataChannel/LibDataChannelTargets.cmake"
         "E:/vsc/BytesScreenShare/build/third_party/libdatachannel/CMakeFiles/Export/32c821eb1e7b36c3a3818aec162f7fd2/LibDataChannelTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/LibDataChannel/LibDataChannelTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/LibDataChannel/LibDataChannelTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/LibDataChannel" TYPE FILE FILES "E:/vsc/BytesScreenShare/build/third_party/libdatachannel/CMakeFiles/Export/32c821eb1e7b36c3a3818aec162f7fd2/LibDataChannelTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/LibDataChannel" TYPE FILE FILES "E:/vsc/BytesScreenShare/build/third_party/libdatachannel/CMakeFiles/Export/32c821eb1e7b36c3a3818aec162f7fd2/LibDataChannelTargets-debug.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/LibDataChannel" TYPE FILE FILES "E:/vsc/BytesScreenShare/build/third_party/libdatachannel/CMakeFiles/Export/32c821eb1e7b36c3a3818aec162f7fd2/LibDataChannelTargets-minsizerel.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/LibDataChannel" TYPE FILE FILES "E:/vsc/BytesScreenShare/build/third_party/libdatachannel/CMakeFiles/Export/32c821eb1e7b36c3a3818aec162f7fd2/LibDataChannelTargets-relwithdebinfo.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/LibDataChannel" TYPE FILE FILES "E:/vsc/BytesScreenShare/build/third_party/libdatachannel/CMakeFiles/Export/32c821eb1e7b36c3a3818aec162f7fd2/LibDataChannelTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/LibDataChannel" TYPE FILE FILES
    "E:/vsc/BytesScreenShare/build/LibDataChannelConfig.cmake"
    "E:/vsc/BytesScreenShare/build/LibDataChannelConfigVersion.cmake"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("E:/vsc/BytesScreenShare/build/third_party/libdatachannel/examples/client/cmake_install.cmake")
  include("E:/vsc/BytesScreenShare/build/third_party/libdatachannel/examples/client-benchmark/cmake_install.cmake")
  include("E:/vsc/BytesScreenShare/build/third_party/libdatachannel/examples/media-receiver/cmake_install.cmake")
  include("E:/vsc/BytesScreenShare/build/third_party/libdatachannel/examples/media-sender/cmake_install.cmake")
  include("E:/vsc/BytesScreenShare/build/third_party/libdatachannel/examples/media-sfu/cmake_install.cmake")
  include("E:/vsc/BytesScreenShare/build/third_party/libdatachannel/examples/streamer/cmake_install.cmake")
  include("E:/vsc/BytesScreenShare/build/third_party/libdatachannel/examples/copy-paste/cmake_install.cmake")
  include("E:/vsc/BytesScreenShare/build/third_party/libdatachannel/examples/copy-paste-capi/cmake_install.cmake")

endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "E:/vsc/BytesScreenShare/build/third_party/libdatachannel/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
