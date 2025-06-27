cmake_minimum_required(VERSION 3.20)

project(obs-multistream VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OBS Studio development environment
find_package(libobs REQUIRED)
find_package(obs-frontend-api REQUIRED)

# Plugin source files
set(PLUGIN_SOURCES
    src/obs-multistream.cpp
    src/multistream-dock.cpp
    src/multistream-output.cpp
)

set(PLUGIN_HEADERS
    src/obs-multistream.h
    src/multistream-dock.h
    src/multistream-output.h
)

# Create the plugin library
add_library(obs-multistream MODULE
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
    src/obs-multistream.rc
)

# Set target properties
set_target_properties(obs-multistream PROPERTIES
    OUTPUT_NAME obs-multistream
    PREFIX ""
)

# Link libraries
target_link_libraries(obs-multistream
    OBS::libobs
    OBS::obs-frontend-api
)

# Include directories
target_include_directories(obs-multistream PRIVATE
    src/
)

# Compiler definitions
target_compile_definitions(obs-multistream PRIVATE
    UNICODE
    _UNICODE
)

# Windows specific settings
if(WIN32)
    target_compile_definitions(obs-multistream PRIVATE
        _WINDOWS
        _USRDLL
        OBSAPIEXPORT_EXPORTS
    )
    
    # Module definition file for exports
    set_target_properties(obs-multistream PROPERTIES
        LINK_FLAGS "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/obs-multistream.def"
    )
endif()

# Installation
if(WIN32)
    install(TARGETS obs-multistream
        RUNTIME DESTINATION "obs-plugins/64bit"
        LIBRARY DESTINATION "obs-plugins/64bit"
    )
endif()

# Package information
set(CPACK_PACKAGE_NAME "obs-multistream")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OBS Studio Multistream Plugin")
set(CPACK_PACKAGE_VENDOR "OBS Multistream Plugin")

include(CPack) 