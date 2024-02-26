cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0042 NEW)

project("nt_native")

add_definitions(-DNAPI_VERSION=4)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
include(~/github/vcpkg/scripts/buildsystems/vcpkg.cmake)

set(PKG_TOOL pnpm)
set(TARGET_ARCH x64)

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

find_package(unofficial-sqlite3 CONFIG REQUIRED)

set(SUBHOOK_TESTS OFF)

# set(SUBHOOK_STATIC ON)
set(SUBHOOK_INSTALL OFF)
add_subdirectory(thirds/subhook)

file(GLOB SOURCE_FILES "src/*.cc" "src/*.hh")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})

target_link_libraries(${PROJECT_NAME} subhook)
target_link_libraries(${PROJECT_NAME} unofficial::sqlite3::sqlite3)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})

################test##################

add_executable(test_sqlite
        test/sqlite3.cc
)
target_link_libraries(test_sqlite PRIVATE unofficial::sqlite3::sqlite3)

