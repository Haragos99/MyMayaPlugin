cmake_minimum_required(VERSION 3.16)
project(MyMayaPlugin LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Maya SDK path 
if(NOT DEFINED MAYA_SDK_ROOT)

    if(WIN32)
        # Look for installed Maya versions in default Windows location
        file(GLOB MAYA_VERSIONS "C:/Program Files/Autodesk/Maya*")
        
        # Sort descending to get the latest version
        list(SORT MAYA_VERSIONS DESCENDING)
        if(MAYA_VERSIONS)
            list(GET MAYA_VERSIONS 0 MAYA_LATEST_PATH)
            set(MAYA_SDK_ROOT "${MAYA_LATEST_PATH}" CACHE PATH "Path to Maya SDK")
        else()
            message(FATAL_ERROR "No Maya installation found in 'C:/Program Files/Autodesk/'. Please set -DMAYA_SDK_ROOT=...")
        endif()

    elseif(UNIX)
        # Look for installed Maya versions in default Linux location
        file(GLOB MAYA_VERSIONS "/usr/autodesk/maya*")
        
        list(SORT MAYA_VERSIONS DESCENDING)
        if(MAYA_VERSIONS)
            list(GET MAYA_VERSIONS 0 MAYA_LATEST_PATH)
            set(MAYA_SDK_ROOT "${MAYA_LATEST_PATH}" CACHE PATH "Path to Maya SDK")
        else()
            message(FATAL_ERROR "No Maya installation found in '/usr/autodesk/'. Please set -DMAYA_SDK_ROOT=...")
        endif()

    endif()

endif()

message(STATUS "Using Maya SDK path: ${MAYA_SDK_ROOT}")

include_directories("${MAYA_SDK_ROOT}/include")
link_directories("${MAYA_SDK_ROOT}/lib")

# Specify that this is a shared library (.dll/.mll)
add_library(MyMayaPlugin SHARED
    src/main.cpp
    src/plugin.cpp
    src/mesh.cpp
    src/deltamush.cpp
    src/deltamushnode.cpp
    src/tight_inclusion/avx.cpp
    src/tight_inclusion/ccd.cpp
    src/tight_inclusion/interval.cpp
    src/tight_inclusion/interval_root_finder.cpp
    src/tight_inclusion/logger.cpp
    src/collison.cpp  "src/debugdraweroverride.cpp" "src/aabb.h")
target_compile_definitions(MyMayaPlugin PRIVATE NOMINMAX)

# If you want to include headers in IDE:
target_sources(MyMayaPlugin PRIVATE
    src/deltamush.h
    src/framework.h
    src/mesh.h
    src/plugin.h
    src/deltamushnode.h
    src/debugdraweroverride.h
    src/collison.h
    src/tight_inclusion/avx.hpp
    src/tight_inclusion/ccd.hpp
    src/tight_inclusion/interval.hpp
    src/tight_inclusion/interval_root_finder.hpp
    src/tight_inclusion/logger.hpp
    src/tight_inclusion/timer.hpp
    src/tight_inclusion/types.hpp
)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)

# Find spdlog
find_package(spdlog REQUIRED)

# Add Eigen include directories to your target
target_link_libraries(MyMayaPlugin PRIVATE Eigen3::Eigen)

target_link_libraries(MyMayaPlugin PRIVATE spdlog::spdlog)


# Build as .mll shared library
set_target_properties(MyMayaPlugin PROPERTIES SUFFIX ".mll")

# Definitions per-configuration
target_compile_definitions(MyMayaPlugin PRIVATE
    MYMAYAPLUGIN_EXPORTS
    _WINDOWS
    _USRDLL
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# Link Maya + system libs
target_link_libraries(MyMayaPlugin PRIVATE
    Foundation.lib
    OpenMaya.lib
    OpenMayaUI.lib
    OpenMayaAnim.lib
    OpenMayaFX.lib
    OpenMayaRender.lib
    Image.lib
    opengl32.lib
)

# Compiler flags
target_compile_options(MyMayaPlugin PRIVATE
    /W3
    /permissive-
)

# Output dirs per-config
set_target_properties(MyMayaPlugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release"
)

# Pre-/Post-build steps
add_custom_command(TARGET MyMayaPlugin PRE_BUILD
    COMMAND "${MAYA_SDK_ROOT}/bin/mayapy.exe" "${CMAKE_CURRENT_SOURCE_DIR}/src/scripts/Unload.py"
)
