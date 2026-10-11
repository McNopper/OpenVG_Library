cmake_minimum_required(VERSION 3.14)

project(OpenVG_Library VERSION 1.0.0 LANGUAGES CXX)

# Build policy + static analysis. Registered ONLY when this is the top-level
# project: EGL is vendored through FetchContent, and a second `cppcheck`
# custom target would make CMake fail that build outright.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
    include(cppcheck)
    include(warnings)
endif()

include(FetchContent)

# EGL: prefer the sibling local checkout (multi-repo development layout), and
# fall back to the published release when it is not present. The OpenVG seam
# is switched on here - it is the platform side of this library and the only
# consumer that needs it.
if(EXISTS "${CMAKE_SOURCE_DIR}/../EGL/CMakeLists.txt")
    FetchContent_Declare(
        egl
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/../EGL
    )
else()
    FetchContent_Declare(
        egl
        GIT_REPOSITORY https://github.com/McNopper/EGL.git
        GIT_TAG        v1.0.14
    )
endif()
set(EGL_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(EGL_ENABLE_OPENVG ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(egl)

if(NOT EGL_ENABLE_OPENVG)
    message(FATAL_ERROR "OpenVG_Library: EGL_ENABLE_OPENVG could not be enabled - the OpenVG seam is required.")
endif()

# The OpenVG 1.1 Reference Implementation by The Khronos Group
# (https://www.khronos.org/openvg/) in RI/ - see THIRD-PARTY.md for what was
# stripped (its mini-EGL and OS layers - that role is played by McNopper/EGL)
# and the two documented header modifications. The seam is src/eglvg_glue.cpp.
set(OPENVG_RI_DIR ${CMAKE_CURRENT_LIST_DIR}/RI)

file(GLOB OPENVG_RI_SOURCES ${OPENVG_RI_DIR}/src/*.cpp)

add_library(OpenVG STATIC
    ${OPENVG_RI_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/src/eglvg_glue.cpp)

# Consumers include <VG/openvg.h>; the implementation's own includes
# ("openvg.h", its ri*.h files) and the EGL provider seam stay private.
target_include_directories(OpenVG
    PUBLIC ${OPENVG_RI_DIR}/include
    PRIVATE ${OPENVG_RI_DIR}/include/VG ${OPENVG_RI_DIR}/src ${egl_SOURCE_DIR}/src)

target_link_libraries(OpenVG PUBLIC EGL)

# OPENVG_STATIC_LIBRARY is the implementation's own switch for static use of
# its public headers (empty VG_API_CALL instead of dllimport). PUBLIC, so
# consumers compiling against <VG/openvg.h> see the same declarations.
target_compile_definitions(OpenVG PUBLIC OPENVG_STATIC_LIBRARY)

# The reference implementation is MSVC6-era C++; keep it at C++14 (throw()
# specifications and other pre-17 constructs).
set_target_properties(OpenVG PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)

if(COMMAND project_enable_warnings)
    project_enable_warnings(OpenVG)
endif()
