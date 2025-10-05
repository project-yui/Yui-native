cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0042 NEW)

project("nt_native")

set(VCPKG_ROOT $ENV{VCPKG_ROOT})
message("VCPKG_ROOT: " ${VCPKG_ROOT})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(PKG_TOOL pnpm)
set(TARGET_ARCH x64)
if(MSVC)
    set(VCPKG_TARGET_TRIPLET x64-windows-static)
    # set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug")
    # set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
endif()
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
include("${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

message("LD_LIBRARY_PATH:" ${LD_LIBRARY_PATH})

find_package(Protobuf CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
if (LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GNUTLS REQUIRED IMPORTED_TARGET gnutls)
endif()

find_program(PNPM_EXECUTABLE pnpm)
if(NOT PNPM_EXECUTABLE)
    message(FATAL_ERROR "pnpm not found!")
endif()
if(WIN32)
    set(PNPM_EXECUTABLE "${PNPM_EXECUTABLE}.CMD")
endif()

# message("安装 Node 包")
# execute_process(COMMAND
#         ${PKG_TOOL} install
#         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
# )
message("CMAKE_CXX_FLAGS:${CMAKE_CXX_FLAGS}")
message("获取 cmake-js 配置")
message(${PROJECT_SOURCE_DIR})
execute_process(COMMAND
    ${PNPM_EXECUTABLE} --silent cmake-js --version
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE CMAKE_JS_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND
    ${PNPM_EXECUTABLE} cmake-js print-cmakejs-src --arch ${TARGET_ARCH} --silent -l error
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE CMAKE_JS_SRC
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message("获取 CMAKE_JS_SRC 配置: ${CMAKE_JS_SRC}")

message("PNPM_EXECUTABLE: " ${PNPM_EXECUTABLE})
execute_process(COMMAND
    ${PNPM_EXECUTABLE} cmake-js print-cmakejs-include --silent -l error
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE CMAKE_JS_INC
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND
    ${PNPM_EXECUTABLE} cmake-js print-cmakejs-lib --silent -l error
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE CMAKE_JS_LIB
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
# string(REPLACE ";" "\;" CMAKE_JS_INC "${CMAKE_JS_INC}")
message("CMAKE_JS_INC:" ${CMAKE_JS_INC})
foreach(item IN LISTS CMAKE_JS_INC)
    message("include: ${item}")
    include_directories(${item})
endforeach()

message("获取 node-addon-api 配置")
execute_process(COMMAND
    node -p "require('node-addon-api').include"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE NODE_ADDON_API_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message("node-addon-api 路径:${NODE_ADDON_API_DIR}")

message("CMAKE_JS_INC: " ${CMAKE_JS_INC})
message("CMAKE_JS_SRC: " ${CMAKE_JS_SRC})
message("CMAKE_JS_LIB: " ${CMAKE_JS_LIB})

include_directories(${CMAKE_JS_SRC})
include_directories(${NODE_ADDON_API_DIR})

set(SUBHOOK_TESTS OFF)
set(SUBHOOK_STATIC ON)
set(SUBHOOK_INSTALL OFF)
add_subdirectory(thirds/subhook)

set(MODULE_NAME nt_native)
add_library(${MODULE_NAME}
    SHARED
    src/include/hook.hh
    src/include/linux_hook.hh
    src/include/windows_hook.hh
    src/include/disasm.hh
    src/main.cc
    src/hook/hook.cc
    src/hook/linux_hook.cc
    src/hook/windows_hook.cc
    src/proto/communication.pb.cc
    src/disasm/custom_disasm.cc
    src/yui/msf_task.cc
    src/yui/msf_worker.cc
    src/install/hook_msf.cc
    ${CMAKE_JS_SRC}
    )
if (WIN32)
    message("生成 QQ.lib")
    execute_process(COMMAND
        ${CMAKE_AR} /def:QQ.def /out:QQ.lib /machine:x86 ${CMAKE_STATIC_LINKER_FLAGS}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/thirds/external
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(COMMAND
        ${CMAKE_AR} /def:QQ64.def /out:QQ64.lib /machine:x64 ${CMAKE_STATIC_LINKER_FLAGS}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/thirds/external
        COMMAND_ERROR_IS_FATAL ANY
    )
    target_link_libraries(${MODULE_NAME} ${PROJECT_SOURCE_DIR}/thirds/external/QQ.lib)
    target_link_libraries(${MODULE_NAME} ${PROJECT_SOURCE_DIR}/thirds/external/QQ64.lib)
endif()

if (WIN32)
    target_link_libraries(${MODULE_NAME} subhook)
    target_link_libraries(${MODULE_NAME} protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
    target_link_libraries(${MODULE_NAME} spdlog::spdlog)
else()
    target_link_libraries(${MODULE_NAME} PRIVATE subhook)
    target_link_libraries(${MODULE_NAME} PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
    target_link_libraries(${MODULE_NAME} PRIVATE spdlog::spdlog)
endif()
target_link_libraries(${MODULE_NAME} ${CMAKE_JS_LIB})

target_compile_definitions(${MODULE_NAME} PRIVATE $<$<CONFIG:Debug>:NATIVE_DEBUG=1>)
set_target_properties(${MODULE_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

################preload###############

add_library(preload
    SHARED
    src/preload.cc
    ${CMAKE_JS_SRC}
    )
    
if (WIN32)
    execute_process(COMMAND
        node tools/windows/download_lib.js
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
    target_link_libraries(preload PRIVATE spdlog::spdlog)
    target_link_libraries(preload PRIVATE ${CMAKE_JS_LIB})
else()
    target_link_libraries(preload PRIVATE spdlog::spdlog)
    target_link_libraries(preload PRIVATE PkgConfig::GNUTLS)
    target_include_directories(preload PRIVATE ${GNUTLS_INCLUDE_DIRS})
    # Ensure we also link with cmake-js / node libs on non-Windows
    target_link_libraries(preload PRIVATE ${CMAKE_JS_LIB})
endif()
set_target_properties(preload PROPERTIES PREFIX "" SUFFIX ".dll")

################QBar###############

add_library(QBar
    SHARED
    src/qbar.cc
    )
    
if (WIN32)
    # Generate node.lib
    target_link_libraries(QBar PRIVATE spdlog::spdlog)
    # If a .def file is present, pass it to the linker so we can forward exports
    set(QBAR_DEF ${PROJECT_SOURCE_DIR}/src/qbar.def)
    if(EXISTS ${QBAR_DEF})
        if(MSVC)
            # For MSVC the /DEF: flag is passed as a LINK_FLAGS property
            set_target_properties(QBar PROPERTIES LINK_FLAGS "/DEF:${QBAR_DEF}")
        else()
            # For other linkers (ld) use the -Wl,--output-def or similar if needed
            target_link_options(QBar PRIVATE "-Wl,--input-def=${QBAR_DEF}")
        endif()
    endif()

endif()
# set output path
set_target_properties(QBar PROPERTIES RUNTIME_OUTPUT_DIRECTORY "D:/Program Files/Tencent/QQNT/resources/app")
set_target_properties(QBar PROPERTIES PREFIX "" SUFFIX ".dll")

################test##################


add_executable(test_protobuf
    test/test_protobuf.cc
    ./src/proto/communication.pb.cc
)
target_link_libraries(test_protobuf PRIVATE spdlog::spdlog)
target_link_libraries(test_protobuf PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)


add_executable(test
        test/test.cc
)
add_executable(test_preload
        test/test_preload.cc
)