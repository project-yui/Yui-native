
################QQNT###############

add_library(QQNT
    SHARED
    ./qqnt.cc
)

target_link_libraries(QQNT PRIVATE spdlog::spdlog)
# Also link the project's QQ64 import library which contains many Cr_z_* exports
# (generated earlier via ${CMAKE_AR}). If present, link it so QQNT resolves zlib wrappers.
target_link_libraries(QQNT PRIVATE ${CMAKE_JS_LIB})
set(QQNT_DEF ${PROJECT_SOURCE_DIR}/src/qqnt/qqnt.def)
if(EXISTS ${QQNT_DEF})
        if(MSVC)
            set_target_properties(QQNT PROPERTIES LINK_FLAGS "/DEF:${QQNT_DEF}")
    else()
        target_link_options(QQNT PRIVATE "-Wl,--input-def=${QQNT_DEF}")
    endif()
else()
    message(FATAL_ERROR "QQNT.def not found")
endif()
# set_target_properties(QQNT PROPERTIES RUNTIME_OUTPUT_DIRECTORY "D:/Program Files/Tencent/QQNT/versions/9.9.21-39038/resources/app")
set_target_properties(QQNT PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_target_properties(QQNT PROPERTIES PREFIX "" SUFFIX ".dll")
