
################preload###############

find_package(PkgConfig REQUIRED)
pkg_check_modules(GNUTLS REQUIRED IMPORTED_TARGET gnutls)

add_library(preload
    SHARED
    ./preload.cc
    ${CMAKE_JS_SRC}
    )
    
target_link_libraries(preload PRIVATE spdlog::spdlog)
target_link_libraries(preload PRIVATE PkgConfig::GNUTLS)
target_include_directories(preload PRIVATE ${GNUTLS_INCLUDE_DIRS})
# Ensure we also link with cmake-js / node libs on non-Windows
target_link_libraries(preload PRIVATE ${CMAKE_JS_LIB})
set_target_properties(preload PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_target_properties(preload PROPERTIES PREFIX "" SUFFIX ".node")
