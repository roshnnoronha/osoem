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

# Find GNU Readline
find_path(READLINE_INCLUDE_DIR
    NAMES readline/readline.h
    PATHS /usr/include /usr/local/include
)

find_library(READLINE_LIBRARY
    NAMES readline
    PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
)

if(READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
    message(STATUS "Found Readline: ${READLINE_LIBRARY}")
else()
    message(FATAL_ERROR "Readline not found. Please install libreadline-dev")
endif()

# Find OpenSSL
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    message(STATUS "Found OpenSSL: ${OPENSSL_VERSION}")
else()
    message(FATAL_ERROR "OpenSSL not found. Please install libssl-dev")
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/src
    ${MYSQLCPPCONN_INCLUDE_DIR}
    ${READLINE_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
)

# Source files
set(SOURCES
    src/main.cpp
    src/data/database.cpp
    src/data/path.cpp
    src/utils/lookup.cpp
    src/data/activitydata.cpp
    src/data/artefactdata.cpp
    src/data/artefacttypedata.cpp
    src/data/categorydata.cpp
    src/data/employeedata.cpp
    src/data/milestonedata.cpp
    src/data/milestonestepdata.cpp
    src/data/notedata.cpp
    src/data/fielddata.cpp
    src/data/projectdata.cpp
    src/data/subcategorydata.cpp
    src/data/taskdata.cpp
    src/data/departmentdata.cpp
    src/data/projectteammemberdata.cpp
    src/data/assignmentdata.cpp
    src/data/hoursdata.cpp
    src/data/artefactdataitemdata.cpp
    src/data/artefacttoactivitylinkdata.cpp
    src/data/milestonetoartefactlinkdata.cpp
    src/commands/execute.cpp
    src/commands/list.cpp
    src/commands/select.cpp
    src/commands/add.cpp
    src/commands/remove.cpp
    src/commands/set.cpp
    src/commands/importcsv.cpp
    src/commands/exportcsv.cpp
    src/commands/help.cpp
    src/utils/csv_parser.cpp
    src/utils/utils.cpp
    src/utils/auth.cpp
    src/utils/completion_provider.cpp
    src/utils/readline_wrapper.cpp
)

# Create executable
add_executable(osoem_admin ${SOURCES})

# Link libraries
target_link_libraries(osoem_admin ${MYSQLCPPCONN_LIBRARY} ${READLINE_LIBRARY} OpenSSL::Crypto)

# Installation
install(TARGETS osoem_admin DESTINATION bin)

# Tests
enable_testing()
add_subdirectory(test)
