
################QQNT###############

add_library(QQNT
    SHARED
    ./qqnt.cc
)

target_link_libraries(QQNT PRIVATE spdlog::spdlog)
# Also link the project's QQ64 import library which contains many Cr_z_* exports
# (generated earlier via ${CMAKE_AR}). If present, link it so QQNT resolves zlib wrappers.
execute_process(COMMAND
    ${CMAKE_AR} /def:QQNT.def /out:QQNT.lib /machine:x64 ${CMAKE_STATIC_LINKER_FLAGS}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/thirds/external
    COMMAND_ERROR_IS_FATAL ANY
)
set(QQNT_LIB ${PROJECT_SOURCE_DIR}/thirds/external/QQNT.lib)
if(EXISTS ${QQNT_LIB})
    target_link_libraries(QQNT PRIVATE ${QQNT_LIB})
else()
    message(FATAL_ERROR "QQNT.lib not found")
endif()
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
# set_target_properties(QQNT PROPERTIES RUNTIME_OUTPUT_DIRECTORY "D:/Program Files/Tencent/QQNT/resources/app")
set_target_properties(QQNT PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_target_properties(QQNT PROPERTIES PREFIX "" SUFFIX ".dll")
