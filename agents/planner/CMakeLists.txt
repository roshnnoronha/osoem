cmake_minimum_required(VERSION 3.14)
project(osoem_planner)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MySQL Connector/C++
find_package(mysqlcppconn QUIET)
if(NOT mysqlcppconn_FOUND)
    find_path(MYSQLCPPCONN_INCLUDE_DIR NAMES mysql_connection.h
        PATHS /usr/include/mysql-cppconn-8 /usr/local/include/mysql-cppconn-8)
    find_library(MYSQLCPPCONN_LIBRARY NAMES mysqlcppconn mysqlcppconn8
        PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu)
    if(NOT MYSQLCPPCONN_INCLUDE_DIR OR NOT MYSQLCPPCONN_LIBRARY)
        message(FATAL_ERROR "MySQL Connector/C++ not found. Install libmysqlcppconn-dev")
    endif()
endif()

# libcurl
find_package(CURL REQUIRED)

# nlohmann/json (header-only)
include(FetchContent)
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Readline
find_path(READLINE_INCLUDE_DIR NAMES readline/readline.h
    PATHS /usr/include /usr/local/include)
find_library(READLINE_LIBRARY NAMES readline
    PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu)
if(NOT READLINE_INCLUDE_DIR OR NOT READLINE_LIBRARY)
    message(FATAL_ERROR "Readline not found. Install libreadline-dev")
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/src
    ${MYSQLCPPCONN_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
    ${READLINE_INCLUDE_DIR}
)

set(SOURCES
    src/main.cpp
    src/data/database.cpp
    src/planner/openai_client.cpp
    src/planner/planner.cpp
    src/planner/agent_tools.cpp
    src/utils/utils.cpp
)

add_executable(osoem_planner ${SOURCES})
target_link_libraries(osoem_planner
    ${MYSQLCPPCONN_LIBRARY}
    CURL::libcurl
    nlohmann_json::nlohmann_json
    ${READLINE_LIBRARY}
)
