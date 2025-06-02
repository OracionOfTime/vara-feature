add_subdirectory(python)
add_library(oxidd INTERFACE)

target_include_directories(oxidd INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)