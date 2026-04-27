cmake_minimum_required(VERSION 3.14)

# ──────────────────────────────────────────────────
# Google Test via FetchContent
# ──────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# Prevent GTest from overriding our compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# ──────────────────────────────────────────────────
# Admin sources library (all src/**/*.cpp except main.cpp)
# ──────────────────────────────────────────────────
set(ADMIN_SOURCES
    ${CMAKE_SOURCE_DIR}/src/data/database.cpp
    ${CMAKE_SOURCE_DIR}/src/data/path.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/lookup.cpp
    ${CMAKE_SOURCE_DIR}/src/data/activitydata.cpp
    ${CMAKE_SOURCE_DIR}/src/data/artefactdata.cpp
    ${CMAKE_SOURCE_DIR}/src/data/artefacttypedata.cpp
    ${CMAKE_SOURCE_DIR}/src/data/categorydata.cpp
    ${CMAKE_SOURCE_DIR}/src/data/employeedata.cpp
    ${CMAKE_SOURCE_DIR}/src/data/milestonedata.cpp
    ${CMAKE_SOURCE_DIR}/src/data/milestonestepdata.cpp
    ${CMAKE_SOURCE_DIR}/src/data/notedata.cpp
    ${CMAKE_SOURCE_DIR}/src/data/fielddata.cpp
    ${CMAKE_SOURCE_DIR}/src/data/projectdata.cpp
    ${CMAKE_SOURCE_DIR}/src/data/subcategorydata.cpp
    ${CMAKE_SOURCE_DIR}/src/data/taskdata.cpp
    ${CMAKE_SOURCE_DIR}/src/data/departmentdata.cpp
    ${CMAKE_SOURCE_DIR}/src/data/projectteammemberdata.cpp
    ${CMAKE_SOURCE_DIR}/src/data/assignmentdata.cpp
    ${CMAKE_SOURCE_DIR}/src/data/hoursdata.cpp
    ${CMAKE_SOURCE_DIR}/src/data/artefactdataitemdata.cpp
    ${CMAKE_SOURCE_DIR}/src/data/artefacttoactivitylinkdata.cpp
    ${CMAKE_SOURCE_DIR}/src/data/milestonetoartefactlinkdata.cpp
    ${CMAKE_SOURCE_DIR}/src/commands/execute.cpp
    ${CMAKE_SOURCE_DIR}/src/commands/list.cpp
    ${CMAKE_SOURCE_DIR}/src/commands/select.cpp
    ${CMAKE_SOURCE_DIR}/src/commands/add.cpp
    ${CMAKE_SOURCE_DIR}/src/commands/remove.cpp
    ${CMAKE_SOURCE_DIR}/src/commands/set.cpp
    ${CMAKE_SOURCE_DIR}/src/commands/importcsv.cpp
    ${CMAKE_SOURCE_DIR}/src/commands/exportcsv.cpp
    ${CMAKE_SOURCE_DIR}/src/commands/help.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/csv_parser.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/utils.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/auth.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/completion_provider.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/readline_wrapper.cpp
)

add_library(osoem_admin_lib STATIC ${ADMIN_SOURCES})

# test/config/ must come BEFORE the parent config/ so our test db_config.h takes precedence
target_include_directories(osoem_admin_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/src
    ${MYSQLCPPCONN_INCLUDE_DIR}
    ${READLINE_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(osoem_admin_lib PUBLIC
    ${MYSQLCPPCONN_LIBRARY}
    ${READLINE_LIBRARY}
    OpenSSL::Crypto
)

# ──────────────────────────────────────────────────
# Macro: add a named test binary
# ──────────────────────────────────────────────────
macro(add_osoem_test name file)
    add_executable(${name} ${file})
    target_link_libraries(${name} PRIVATE
        osoem_admin_lib
        GTest::gtest
        GTest::gtest_main
    )
    target_include_directories(${name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}        # for test_helpers.h
        ${CMAKE_CURRENT_SOURCE_DIR}/config # test db_config.h
        ${CMAKE_SOURCE_DIR}/src
        ${MYSQLCPPCONN_INCLUDE_DIR}
        ${READLINE_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIR}
    )
    # WORKING_DIRECTORY set to CMAKE_SOURCE_DIR (admin/) so relative paths like
    # "examples/import/..." work correctly inside tests.
    add_test(NAME ${name} COMMAND ${name} --gtest_color=yes
             WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endmacro()

# ──────────────────────────────────────────────────
# Test targets
# ──────────────────────────────────────────────────
add_osoem_test(test_utils          test_utils.cpp)
add_osoem_test(test_csv_parser     test_csv_parser.cpp)
add_osoem_test(test_path           test_path.cpp)
add_osoem_test(test_auth           test_auth.cpp)
add_osoem_test(test_input_parsing  test_input_parsing.cpp)
add_osoem_test(test_navigation     test_navigation.cpp)
add_osoem_test(test_list           test_list.cpp)
add_osoem_test(test_add            test_add.cpp)
add_osoem_test(test_remove         test_remove.cpp)
add_osoem_test(test_set            test_set.cpp)
add_osoem_test(test_import         test_import.cpp)
add_osoem_test(test_export         test_export.cpp)
add_osoem_test(test_batch          test_batch.cpp)
add_osoem_test(test_data_integrity test_data_integrity.cpp)
add_osoem_test(test_help           test_help.cpp)
