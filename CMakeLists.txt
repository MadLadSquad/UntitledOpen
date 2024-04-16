cmake_minimum_required(VERSION 3.21)

project(UntitledOpen VERSION 1.0.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

include_directories(. NFD/src/include/)

set(UOPEN_LIBRARY_TYPE "SHARED")
if (BUILD_VARIANT_STATIC)
    set(UOPEN_LIBRARY_TYPE "STATIC")
    set(BUILD_SHARED_LIBS OFF)
endif()
add_subdirectory(NFD)

set(BUILD_SHARED_LIBS OFF)

add_library(UntitledOpen ${UOPEN_LIBRARY_TYPE} C/CUntitledOpen.c C/CUntitledOpen.h Common.h UntitledOpen.cpp UntitledOpen.hpp)

target_compile_definitions(UntitledOpen PRIVATE UVK_LOG_EXPORT_FROM_LIBRARY UVK_LIB_COMPILE)
target_compile_definitions(UntitledOpen PUBLIC UIMGUI_OPEN_SUBMODULE_ENABLED)
list(APPEND compile_defs "UIMGUI_OPEN_SUBMODULE_ENABLED")

target_link_libraries(UntitledOpen nfd)

configure_file(UntitledOpen.pc.in UntitledOpen.pc @ONLY)

if (UIMGUI_INSTALL)
    install(TARGETS UntitledOpen
            LIBRARY DESTINATION lib/
            ARCHIVE DESTINATION lib/)
    install(FILES UntitledOpen.h Common.h DESTINATION include/UntitledOpen)
    install(FILES C/CUntitledOpen.h DESTINATION include/UntitledOpen/C)
    install(FILES ${PROJECT_BINARY_DIR}/UntitledOpen.pc
            DESTINATION lib/pkgconfig/)
endif()
