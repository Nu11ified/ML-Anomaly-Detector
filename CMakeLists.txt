cmake_minimum_required(VERSION 3.10)
project(AnomDetect CXX)

set(CMAKE_CXX_STANDARD 17)

# Platform detection
if(WIN32)
    set(PLATFORM "windows")
    set(PLATFORM_SOURCES src/platform_windows.cpp)
    set(PLATFORM_LIBS pdh.lib psapi.lib)
elseif(APPLE)
    set(PLATFORM "macos")
    set(PLATFORM_SOURCES src/platform_macos.cpp)
    set(PLATFORM_LIBS "")
elseif(UNIX)
    set(PLATFORM "linux")
    set(PLATFORM_SOURCES src/platform_linux.cpp)
    set(PLATFORM_LIBS "")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

message(STATUS "Building for platform: ${PLATFORM}")

include_directories(include)

# Common source files
set(COMMON_SOURCES
    src/main.cpp
    src/cli_monitor_impl.cpp
    src/platform_factory.cpp
)

# Platform-specific executable
add_executable(anom_detect_${PLATFORM}
    ${COMMON_SOURCES}
    ${PLATFORM_SOURCES}
)

# Link platform-specific libraries
if(PLATFORM_LIBS)
    target_link_libraries(anom_detect_${PLATFORM} ${PLATFORM_LIBS})
endif()

# Set output directory
set_target_properties(anom_detect_${PLATFORM} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Create a generic executable name for the current platform
add_executable(anom_detect ALIAS anom_detect_${PLATFORM})

# Installation
install(TARGETS anom_detect_${PLATFORM}
    RUNTIME DESTINATION bin
)

# Package configuration
set(CPACK_PACKAGE_NAME "anom-detect")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "System Anomaly Detector")
set(CPACK_PACKAGE_VENDOR "AnomDetect Team")

if(WIN32)
    set(CPACK_GENERATOR "ZIP")
    set(CPACK_PACKAGE_FILE_NAME "anom-detect-${CPACK_PACKAGE_VERSION}-windows")
elseif(APPLE)
    set(CPACK_GENERATOR "ZIP")
    set(CPACK_PACKAGE_FILE_NAME "anom-detect-${CPACK_PACKAGE_VERSION}-macos")
elseif(UNIX)
    set(CPACK_GENERATOR "TGZ")
    set(CPACK_PACKAGE_FILE_NAME "anom-detect-${CPACK_PACKAGE_VERSION}-linux")
endif()

include(CPack)