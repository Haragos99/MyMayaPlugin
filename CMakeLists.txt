cmake_minimum_required(VERSION 3.16)
project(MyMayaPlugin LANGUAGES CXX)

# 1. C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Maya SDK path 
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
    src/dllmain.cpp
    src/plugin.cpp
    src/mesh.cpp
    src/deltamush.cpp
)

# If you want to include headers in IDE:
target_sources(MyMayaPlugin PRIVATE
    src/deltamush.h
    src/framework.h
    src/mesh.h
    src/plugin.h
)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)

# Add Eigen include directories to your target
target_link_libraries(MyMayaPlugin PRIVATE Eigen3::Eigen)


# 4. Build as .mll shared library
set_target_properties(MyMayaPlugin PROPERTIES SUFFIX ".mll")

# 5. Definitions per-configuration
target_compile_definitions(MyMayaPlugin PRIVATE
    MYMAYAPLUGIN_EXPORTS
    _WINDOWS
    _USRDLL
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# 6. Link Maya + system libs
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

# 7. Compiler flags
target_compile_options(MyMayaPlugin PRIVATE
    /W3
    /permissive-
)

# 8. Output dirs per-config
set_target_properties(MyMayaPlugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release"
)

# 9. Pre-/Post-build steps
add_custom_command(TARGET MyMayaPlugin PRE_BUILD
    COMMAND "${MAYA_SDK_ROOT}/bin/mayapy.exe" "${CMAKE_CURRENT_SOURCE_DIR}/src/scripts/Unload.py"
)
#add_custom_command(TARGET MyMayaPlugin POST_BUILD
 #   COMMAND ${CMAKE_COMMAND} -E copy
  #          "$<TARGET_FILE:MyMayaPlugin>"
   #         "${MAYA_SDK_ROOT}/plug-ins/"
#)
