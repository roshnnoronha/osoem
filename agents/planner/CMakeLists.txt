cmake_minimum_required(VERSION 3.14)
project(osoem_planner)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Code shared across agents (data/, client/, exceptions/, utils/) and the
# generated config headers live one level up, alongside this agent's folder.
set(AGENTS_ROOT ${CMAKE_SOURCE_DIR}/..)

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
    ${AGENTS_ROOT}/config
    ${AGENTS_ROOT}/include
    ${AGENTS_ROOT}/include/data
    ${AGENTS_ROOT}/include/client
    ${AGENTS_ROOT}/include/exceptions
    ${AGENTS_ROOT}/include/utils
    ${MYSQLCPPCONN_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
    ${READLINE_INCLUDE_DIR}
)

set(COMMON_SOURCES
    ${AGENTS_ROOT}/include/data/database.cpp
    ${AGENTS_ROOT}/include/client/openai_client.cpp
    ${AGENTS_ROOT}/include/utils/utils.cpp
)

add_executable(osoem_planner
    main.cpp
    ${COMMON_SOURCES}
    planner/planner.cpp
    planner/planner_tools.cpp
)
target_link_libraries(osoem_planner
    ${MYSQLCPPCONN_LIBRARY}
    CURL::libcurl
    nlohmann_json::nlohmann_json
    ${READLINE_LIBRARY}
)

