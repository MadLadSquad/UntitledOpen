cmake_minimum_required(VERSION 3.21)

project(UntitledOpen VERSION 2.0.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

include_directories(. NFD/src/include/)

set(UOPEN_LIBRARY_TYPE "SHARED")
if (BUILD_VARIANT_STATIC)
    set(UOPEN_LIBRARY_TYPE "STATIC")
endif()
add_subdirectory(NFD)

if (NOT WIN32 AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(DBUS REQUIRED dbus-1)
    include_directories(${DBUS_INCLUDE_DIRS})
endif()

add_library(UntitledOpen ${UOPEN_LIBRARY_TYPE} C/CUntitledOpen.cpp C/CUntitledOpen.h Common.h UntitledOpen.cpp UntitledOpen.hpp)

target_compile_definitions(UntitledOpen PRIVATE UVK_LOG_EXPORT_FROM_LIBRARY UVK_LIB_COMPILE)
target_compile_definitions(UntitledOpen PUBLIC UIMGUI_OPEN_SUBMODULE_ENABLED)
list(APPEND compile_defs "UIMGUI_OPEN_SUBMODULE_ENABLED")

target_link_libraries(UntitledOpen nfd)
if (APPLE)
    target_link_libraries(UntitledOpen "-framework CoreFoundation" "-framework ApplicationServices")
elseif (NOT WIN32)
    target_link_libraries(UntitledOpen ${DBUS_LINK_LIBRARIES})
endif()

configure_file(UntitledOpen.pc.in UntitledOpen.pc @ONLY)

if (UIMGUI_INSTALL)
    install(TARGETS UntitledOpen
            LIBRARY DESTINATION lib64/
            ARCHIVE DESTINATION lib64/)
    install(FILES UntitledOpen.hpp Common.h DESTINATION include/UntitledOpen)
    install(FILES C/CUntitledOpen.h DESTINATION include/UntitledOpen/C)
    install(FILES ${PROJECT_BINARY_DIR}/UntitledOpen.pc
            DESTINATION lib/pkgconfig/)
endif()
