#pragma once

#include <gtest/gtest.h>

#include "data/database.h"
#include "data/path.h"
#include "commands/execute.h"
#include "exceptions/exceptions.h"

#include <string>
#include <initializer_list>
#include <cstdlib>
#include <stdexcept>
#include <sstream>

using namespace data;

// ──────────────────────────────────────────────────────────────────────────────
// OsoemTestFixture
//
// Base class for all osoem integration tests.  Each test suite that derives
// from this class shares a single Database connection (opened once in
// SetUpTestSuite) and resets the database to a known state before each suite.
// Individual tests get a fresh Path object (reset to root) via SetUp().
// ──────────────────────────────────────────────────────────────────────────────
class OsoemTestFixture : public ::testing::Test {
protected:
    // Shared DB connection — created once per test suite
    static data::Database* db;

    // Per-test path, reset to root before each test
    data::Path pth;

    // ── Suite-level setup / teardown ──────────────────────────────────────────

    static void SetUpTestSuite() {
        db = new data::Database();
        resetAndSeedTestDb();
    }

    static void TearDownTestSuite() {
        delete db;
        db = nullptr;
    }

    // ── Per-test setup ────────────────────────────────────────────────────────

    void SetUp() override {
        pth.reset();
    }

    // ── Helpers ───────────────────────────────────────────────────────────────

    // Run a command string and capture its stdout output.
    // isAdmin defaults to true (matches batch-mode behaviour).
    // Returns empty string on silent commands (add/remove/set etc. print to stdout).
    std::string runCommand(const std::string& cmd, bool isAdmin = true) {
        testing::internal::CaptureStdout();
        commands::execute(*db, pth, cmd, isAdmin);
        return testing::internal::GetCapturedStdout();
    }

    // Navigate to a sequence of items by issuing `select` commands.
    // Each string is passed verbatim to `commands::select`.
    void navigateTo(std::initializer_list<std::string> steps) {
        for (const auto& step : steps) {
            commands::select(*db, pth, step);
        }
    }

    // Execute SQL and return the first column of the first row as an integer.
    // Returns 0 if the result set is empty.
    int queryCount(const std::string& sql) {
        auto rs = db->executeQuery(sql);
        if (rs && rs->next()) {
            return rs->getInt(1);
        }
        return 0;
    }

    // Execute SQL and return the first column of the first row as a string.
    // Returns empty string if the result set is empty.
    std::string queryString(const std::string& sql) {
        auto rs = db->executeQuery(sql);
        if (rs && rs->next()) {
            return rs->getString(1);
        }
        return "";
    }

    // Reset the test database to the known schema-only state, then seed it
    // with departments/employees and the residential building project.
    //
    // The reset SQL is sourced from ../database/reset_database.sql but we
    // substitute the database name so it operates on osoem_test, not trial.
    static void resetAndSeedTestDb() {
        // Use mysql CLI to drop/recreate the test database and apply schema.
        // The reset_database.sql hard-codes "trial"; we override with a
        // USE statement issued before sourcing schema.sql directly.
        const std::string schemaFile = "../database/schema/schema.sql";
        const std::string dbName = "osoem_test";
        const std::string user = "roshn";
        const std::string pass = "p@ssword";

        // Build shell command that:
        //   1. Drops and recreates osoem_test
        //   2. Sources the schema SQL
        std::string cmd =
            "mysql -u" + user + " -p" + pass +
            " -e \"DROP DATABASE IF EXISTS " + dbName + "; "
            "CREATE DATABASE " + dbName + " CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;\" "
            "&& mysql -u" + user + " -p" + pass + " " + dbName +
            " < " + schemaFile;

        int ret = std::system(cmd.c_str());
        if (ret != 0) {
            throw std::runtime_error(
                "resetAndSeedTestDb: failed to reset osoem_test schema (exit " +
                std::to_string(ret) + ")");
        }

        // Seed departments and employees
        data::Path seedPath;
        commands::batchExecute(*db, seedPath, "examples/Organization_Examples/departments.src");

        // Seed the residential building project
        seedPath.reset();
        commands::batchExecute(*db, seedPath, "examples/Residential_Building_Project/project.src");
    }
};

// Static member definition — each translation unit that includes this header
// only needs the declaration; exactly ONE .cpp file must provide the definition.
// We use an inline variable (C++17) so each test binary gets exactly one copy.
inline data::Database* OsoemTestFixture::db = nullptr;
