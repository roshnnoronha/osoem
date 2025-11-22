cmake_minimum_required(VERSION 3.10)
project(osoem_admin)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find MySQL Connector/C++
find_package(mysqlcppconn QUIET)

if(NOT mysqlcppconn_FOUND)
    # Try alternative approach - find the library manually
    find_path(MYSQLCPPCONN_INCLUDE_DIR
        NAMES mysql_connection.h
        PATHS /usr/include/mysql-cppconn-8 /usr/local/include/mysql-cppconn-8
    )

    find_library(MYSQLCPPCONN_LIBRARY
        NAMES mysqlcppconn mysqlcppconn8
        PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
    )

    if(MYSQLCPPCONN_INCLUDE_DIR AND MYSQLCPPCONN_LIBRARY)
        set(MYSQLCPPCONN_FOUND TRUE)
        message(STATUS "Found MySQL Connector/C++: ${MYSQLCPPCONN_LIBRARY}")
    else()
        message(FATAL_ERROR "MySQL Connector/C++ not found. Please install libmysqlcppconn-dev")
    endif()
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/src
    ${MYSQLCPPCONN_INCLUDE_DIR}
)

# Source files
set(SOURCES
    src/main.cpp
    src/database.cpp
    src/csv_parser.cpp
    src/utils.cpp
    src/path.cpp
    src/commands/list.cpp
    src/commands/select.cpp
    src/commands/add.cpp
    src/commands/remove.cpp
    src/commands/employee.cpp
)

# Create executable
add_executable(osoem_admin ${SOURCES})

# Link libraries
target_link_libraries(osoem_admin ${MYSQLCPPCONN_LIBRARY})

# Installation
install(TARGETS osoem_admin DESTINATION bin)
