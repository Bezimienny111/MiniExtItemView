cmake_minimum_required(VERSION 3.21)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(MiniExtItemView VERSION 0.1.0.0 LANGUAGES CXX RC)

if(NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "Use the MSVC x86 toolchain (Win32), not x64 or MinGW.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(GENERATE_UNION_API_DLL ON CACHE BOOL "" FORCE)
set(GENERATE_UNION_API_LIB OFF CACHE BOOL "" FORCE)
foreach(engine G1 G1A G2 G2A)
    set(GOTHIC_API_${engine} ON CACHE BOOL "" FORCE)
endforeach()
add_subdirectory(dependencies/union)

include(generate_ipp_translation_units)
generate_ipp_translation_units(
    INPUT "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/miniExtItemView/MiniExtItemViewModule.ipp"
    OUTPUT PLUGIN_UNITS
    ENGINES G1 Gothic_I_Classic G1A Gothic_I_Addon G2 Gothic_II_Classic G2A Gothic_II_Addon)
add_library(MiniExtItemView SHARED src/DllMain.cpp ${PLUGIN_UNITS})
target_compile_definitions(MiniExtItemView PRIVATE MINI_EXT_ITEM_VIEW_PLUGIN)
target_compile_options(MiniExtItemView PRIVATE /utf-8)
target_include_directories(MiniExtItemView PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(MiniExtItemView PRIVATE union)
include(target_add_versioninfo)
target_add_versioninfo(MiniExtItemView VERSION "${PROJECT_VERSION}"
    COMPANY_NAME "Gothic Community" PRODUCT_NAME "MiniExtItemView")

option(BUILD_VDF "Package the plugin using an external GothicVDFS installation" ON)
set(GOTHIC_VDFS_EXECUTABLE "" CACHE FILEPATH "Path to GothicVDFS.exe (not redistributed here)")
if(BUILD_VDF)
    if(NOT EXISTS "${GOTHIC_VDFS_EXECUTABLE}")
        message(FATAL_ERROR "Set GOTHIC_VDFS_EXECUTABLE to GothicVDFS.exe, or BUILD_VDF=OFF for DLL-only builds.")
    endif()
    set(VDF_DIR "${CMAKE_CURRENT_BINARY_DIR}/package")
    file(MAKE_DIRECTORY "${VDF_DIR}/System/Autorun")
    configure_file("mini-ext-item-view-vdf/build.vm" "${VDF_DIR}/build.vm")
    set(MENU_DEPENDENCY)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/mini-ext-item-view-vdf/System/Autorun/IAI.d")
        configure_file("mini-ext-item-view-vdf/System/Autorun/IAI.d" "${VDF_DIR}/System/Autorun/IAI.d" COPYONLY)
        set(MENU_DEPENDENCY "${VDF_DIR}/System/Autorun/IAI.d")
    endif()
    add_custom_command(
        OUTPUT "${VDF_DIR}/MiniExtItemView.vdf"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:MiniExtItemView>" "${VDF_DIR}/System/Autorun/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:union_api_dll>" "${VDF_DIR}/System/Autorun/"
        COMMAND ${CMAKE_COMMAND}
            "-DVDF_EXECUTABLE=${GOTHIC_VDFS_EXECUTABLE}"
            "-DVDF_SCRIPT=${VDF_DIR}/build.vm"
            "-DVDF_OUTPUT=${VDF_DIR}/MiniExtItemView.vdf"
            "-DVDF_WORKING_DIRECTORY=${VDF_DIR}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/run_gothic_vdfs.cmake"
        DEPENDS MiniExtItemView union_api_dll "${VDF_DIR}/build.vm" ${MENU_DEPENDENCY}
            "${GOTHIC_VDFS_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/run_gothic_vdfs.cmake"
        WORKING_DIRECTORY "${VDF_DIR}"
        VERBATIM)
    add_custom_target(build_vdf ALL DEPENDS "${VDF_DIR}/MiniExtItemView.vdf")
endif()
