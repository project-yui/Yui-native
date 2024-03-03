cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0042 NEW)

project("nt_native")

set(VCPKG_ROOT $ENV{VCPKG_ROOT})
message("VCPKG_ROOT: " ${VCPKG_ROOT})

set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

add_definitions(-DNAPI_VERSION=4)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(VCPKG_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

set(PKG_TOOL pnpm)
set(TARGET_ARCH x64)

message("LD_LIBRARY_PATH:" ${LD_LIBRARY_PATH})

find_package(protobuf CONFIG REQUIRED)
find_package(SqliteOrm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# message("安装 Node 包")
# execute_process(COMMAND
#         ${PKG_TOOL} install
#         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
# )
message("CMAKE_CXX_FLAGS:${CMAKE_CXX_FLAGS}")
message("获取 cmake-js 配置")
message(${PROJECT_SOURCE_DIR})
execute_process(COMMAND
        ${PKG_TOOL} --silent cmake-js --version
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE CMAKE_JS_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND
        ${PKG_TOOL} --silent cmake-js print-cmakejs-src --arch ${TARGET_ARCH}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE CMAKE_JS_SRC
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND
        ${PKG_TOOL} --silent cmake-js print-cmakejs-include --arch ${TARGET_ARCH}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE CMAKE_JS_INC
        OUTPUT_STRIP_TRAILING_WHITESPACE
)


message("获取 node-addon-api 配置")
execute_process(COMMAND
        node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
message("node-addon-api 路径:${NODE_ADDON_API_DIR}")

message("CMAKE_JS_INC: ${CMAKE_JS_INC}")
include_directories(${CMAKE_JS_INC})
include_directories(${NODE_ADDON_API_DIR})

set(SUBHOOK_TESTS OFF)
set(SUBHOOK_STATIC ON)
set(SUBHOOK_INSTALL OFF)
add_subdirectory(thirds/subhook)

message("CMAKE_JS_SRC: ${CMAKE_JS_SRC}")

set(MODULE_NAME nt_native)
add_library(${MODULE_NAME}
        SHARED
        src/include/hook.hh
        src/include/linux_hook.hh
        src/include/db/group_msg_table.hh
        src/proto/message.pb.h
        src/main.cc
        src/hook/hook.cc
        src/hook/linux_hook.cc
        src/proto/message.pb.cc
        src/test/test_sqlite3.cc
        src/db/group_msg_table.cc
        src/proto/message.pb.cc
        src/yukihana/handle.cc
        src/include/db/model/group_msg_table.hh
        ${CMAKE_JS_SRC}
        )
# target_link_libraries(${MODULE_NAME} PRIVATE unofficial::sqlite3::sqlite3)
target_link_libraries(${MODULE_NAME} PRIVATE subhook)
target_link_libraries(${MODULE_NAME} PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
target_link_libraries(${MODULE_NAME} PRIVATE sqlite_orm::sqlite_orm)
target_link_libraries(${MODULE_NAME} PRIVATE spdlog::spdlog)
target_compile_definitions(${MODULE_NAME} PRIVATE $<$<CONFIG:Debug>:NATIVE_DEBUG=1>)

set_target_properties(${MODULE_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
message("CMAKE_JS_LIB:"${CMAKE_JS_LIB})
target_link_libraries(${MODULE_NAME} ${CMAKE_JS_LIB})

################test##################

add_executable(test_sqlite
        test/test_sqlite3.cc
)
target_link_libraries(test_sqlite PRIVATE unofficial::sqlite3::sqlite3)
# target_link_libraries(test_sqlite PRIVATE sqlite3)


add_executable(test_protobuf
        test/test_protobuf.cc
        ./src/proto/message.pb.cc
)
# target_link_libraries(test_sqlite PRIVATE unofficial::sqlite3::sqlite3)
target_link_libraries(test_protobuf PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)



add_executable(test_orm
        test/test_orm.cc
        src/hook/hook.cc
        src/hook/linux_hook.cc
        src/yukihana/handle.cc
        src/db/group_msg_table.cc
        src/proto/message.pb.cc
)
# target_link_libraries(test_sqlite PRIVATE unofficial::sqlite3::sqlite3)
target_link_libraries(test_orm PRIVATE sqlite_orm::sqlite_orm)
target_link_libraries(test_orm PRIVATE subhook)
target_link_libraries(test_orm PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)

