cmake_minimum_required(VERSION 3.4)

project(olm VERSION 3.2.16 LANGUAGES CXX C)

option(OLM_TESTS "Build tests" ON)
option(BUILD_SHARED_LIBS "Build as a shared library" ON)

add_definitions(-DOLMLIB_VERSION_MAJOR=${PROJECT_VERSION_MAJOR})
add_definitions(-DOLMLIB_VERSION_MINOR=${PROJECT_VERSION_MINOR})
add_definitions(-DOLMLIB_VERSION_PATCH=${PROJECT_VERSION_PATCH})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_library(olm
    src/account.cpp
    src/base64.cpp
    src/cipher.cpp
    src/crypto.cpp
    src/memory.cpp
    src/message.cpp
    src/pickle.cpp
    src/ratchet.cpp
    src/session.cpp
    src/utility.cpp
    src/pk.cpp
    src/sas.c

    src/ed25519.c
    src/error.c
    src/inbound_group_session.c
    src/megolm.c
    src/olm.cpp
    src/outbound_group_session.c
    src/pickle_encoding.c

    lib/crypto-algorithms/aes.c
    lib/crypto-algorithms/sha256.c
    lib/curve25519-donna/curve25519-donna.c)
add_library(Olm::Olm ALIAS olm)

# restrict the exported symbols
include(GenerateExportHeader)
generate_export_header(olm
  EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/olm/olm_export.h)

target_include_directories(olm
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib)

set_target_properties(olm PROPERTIES
   SOVERSION ${PROJECT_VERSION_MAJOR}
   VERSION ${PROJECT_VERSION})

set_target_properties(olm PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(GNUInstallDirs)

# Make a pkg-config file
configure_file(${PROJECT_NAME}.pc.in ${PROJECT_NAME}.pc @ONLY NEWLINE_STYLE UNIX)

#
# Installation
#
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/Olm)
install(TARGETS olm
    EXPORT olm-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# The exported target will be named Olm.
set_target_properties(olm PROPERTIES EXPORT_NAME Olm)
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/olm/olm.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/olm/olm_export.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/olm/outbound_group_session.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/olm/inbound_group_session.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/olm/pk.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/olm/sas.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/olm/error.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/olm)

if (UNIX AND NOT APPLE)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
endif ()

# Export the targets to a script.
install(EXPORT olm-targets
  FILE OlmTargets.cmake
  NAMESPACE Olm::
  DESTINATION ${INSTALL_CONFIGDIR})

# Create a ConfigVersion.cmake file.
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/OlmConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/OlmConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/OlmConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR})

#Install the config & configversion.
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/OlmConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/OlmConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR})

# Register package in user's package registry
export(EXPORT olm-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/OlmTargets.cmake
    NAMESPACE Olm::)
export(PACKAGE Olm)

if (OLM_TESTS)
   add_subdirectory(tests)
endif()
