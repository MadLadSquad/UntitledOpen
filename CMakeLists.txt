cmake_minimum_required(VERSION 3.21)

project(UntitledOpen VERSION 1.0.0.0)

set(CMAKE_C_STANDARD 99)

include_directories(.)

set(UOPEN_LIBRARY_TYPE "SHARED")
if (BUILD_VARIANT_STATIC)
    set(UOPEN_LIBRARY_TYPE "STATIC")
endif()

add_library(UntitledOpen ${UOPEN_LIBRARY_TYPE} UntitledOpen.c UntitledOpen.h)

target_compile_definitions(UntitledOpen PRIVATE UVK_LOG_EXPORT_FROM_LIBRARY UVK_LIB_COMPILE)
target_compile_definitions(UntitledOpen PUBLIC UIMGUI_OPEN_SUBMODULE_ENABLED)
list(APPEND compile_defs "UIMGUI_OPEN_SUBMODULE_ENABLED")

target_link_directories(UntitledOpen PUBLIC "./Bindings/target/release/")
if (NOT WIN32)
    target_link_libraries(UntitledOpen UntitledOpenBindings pthread dl)
endif()

configure_file(UntitledOpen.pc.in UntitledOpen.pc @ONLY)

if (UIMGUI_INSTALL)
    install(TARGETS UntitledOpen
            LIBRARY DESTINATION lib/
            ARCHIVE DESTINATION lib/)
    install(FILES UntitledOpen.h DESTINATION include/UntitledOpen)
    install(FILES ${PROJECT_BINARY_DIR}/UntitledOpen.pc
            DESTINATION lib/pkgconfig/)
endif()
