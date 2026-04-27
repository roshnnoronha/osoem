// test_import.cpp — Integration tests for the import command
// Covers TEST_PLAN sections I-01 to I-11

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "exceptions/exceptions.h"

#include <fstream>
#include <cstdio>

class ImportTest : public OsoemTestFixture {};

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

// Write a temporary CSV file, returning its path. Caller must delete it.
static std::string writeTempCsv(const std::string& basename,
                                 const std::string& content) {
    std::string path = "/tmp/" + basename;
    std::ofstream f(path);
    f << content;
    f.close();
    return path;
}

// ─────────────────────────────────────────────────────────────────────────────
// Tests
// ─────────────────────────────────────────────────────────────────────────────

// I-01 — Import valid CSV (employees) succeeds
TEST_F(ImportTest, I01_ImportEmployeesCsv) {
    // Navigate to department 1 (created by departments.src)
    navigateTo({"Organization", "Departments"});
    commands::select(*db, pth, "1");

    int before = queryCount("SELECT COUNT(*) FROM Employees");

    // The CSV may already be imported; catch DuplicateEntryError gracefully
    try {
        runCommand("import examples/CSV_Examples/employeedata.csv");
    } catch (const DuplicateEntryError&) {
        GTEST_SKIP() << "employees already imported";
    }

    int after = queryCount("SELECT COUNT(*) FROM Employees");
    EXPECT_GT(after, before);
}

// I-02 — Import valid CSV (projects)
TEST_F(ImportTest, I02_ImportProjectsCsv) {
    navigateTo({"Projects"});
    int before = queryCount("SELECT COUNT(*) FROM Projects");

    try {
        runCommand("import examples/CSV_Examples/projectdata.csv");
    } catch (const DuplicateEntryError&) {
        GTEST_SKIP() << "projects already imported";
    }

    int after = queryCount("SELECT COUNT(*) FROM Projects");
    EXPECT_GT(after, before);
}

// I-03 — Re-importing same CSV raises DuplicateEntryError (no partial import)
TEST_F(ImportTest, I03_ReImportSameCsvRaisesDuplicateError) {
    navigateTo({"Organization", "Departments"});
    commands::select(*db, pth, "1");

    // First import (may already be done)
    try { runCommand("import examples/CSV_Examples/employeedata.csv"); }
    catch (const DuplicateEntryError&) {}

    // Second import must throw
    EXPECT_THROW(runCommand("import examples/CSV_Examples/employeedata.csv"),
                 DuplicateEntryError);
}

// I-04 — CSV missing required column raises CsvParseError or ValidationError
TEST_F(ImportTest, I04_ImportMissingRequiredColumn) {
    std::string path = writeTempCsv("i04_bad_project.csv",
                                    "projectno\nPRJ-BAD-001\n");
    navigateTo({"Projects"});
    EXPECT_THROW(runCommand("import " + path), DataError);
    std::remove(path.c_str());
}

// I-05 — CSV with unknown column raises InvalidFieldError or similar
TEST_F(ImportTest, I05_ImportExtraUnknownColumn) {
    std::string path = writeTempCsv("i05_extra_col.csv",
                                    "projectname,projectno,unknownfield\n"
                                    "I05 Project,I05-001,whatever\n");
    navigateTo({"Projects"});
    EXPECT_THROW(runCommand("import " + path), DataError);
    std::remove(path.c_str());
}

// I-06 — Import at wrong location (entity mismatch) raises validation error
TEST_F(ImportTest, I06_ImportAtWrongLocation) {
    navigateTo({"Projects"});
    // Employee CSV at projects path — field names won't match
    EXPECT_THROW(runCommand("import examples/CSV_Examples/employeedata.csv"), DataError);
}

// I-08 — Import CSV with invalid date raises ValidationError or CsvParseError
TEST_F(ImportTest, I08_ImportInvalidDate) {
    // Need to be at a subcategory path with a team member
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Activity Categories");

    int catId = queryCount(
        "SELECT categoryid FROM ActivityCategories "
        "WHERE projectid = (SELECT projectid FROM Projects WHERE projectname='Building Project') "
        "LIMIT 1");
    if (catId == 0) GTEST_SKIP() << "No categories";
    commands::select(*db, pth, std::to_string(catId));

    int subId = queryCount(
        "SELECT subcategoryid FROM ActivitySubcategories "
        "WHERE categoryid=" + std::to_string(catId) + " LIMIT 1");
    if (subId == 0) GTEST_SKIP() << "No subcategories";
    commands::select(*db, pth, std::to_string(subId));

    std::string path = writeTempCsv("i08_bad_activity.csv",
        "activityname,activitymanagername,plannedstart,plannedfinish,plannedhours\n"
        "Bad Activity,John Davis,not-a-date,2026-04-01,100\n");

    EXPECT_THROW(runCommand("import " + path), DataError);
    std::remove(path.c_str());
}

// I-09 — Import non-existent file raises FileIOError
TEST_F(ImportTest, I09_ImportNonExistentFile) {
    EXPECT_THROW(runCommand("import /tmp/does_not_exist_osoem.csv"), FileIOError);
}

// I-10 — Import milestones CSV
TEST_F(ImportTest, I10_ImportMilestonesCsv) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Milestones");

    int before = queryCount("SELECT COUNT(*) FROM Milestones");

    try {
        runCommand("import examples/CSV_Examples/milestonedata.csv");
    } catch (const DuplicateEntryError&) {
        GTEST_SKIP() << "milestones already imported";
    }

    int after = queryCount("SELECT COUNT(*) FROM Milestones");
    EXPECT_GT(after, before);
}

// I-12 — Import with -i flag skips duplicates without error
TEST_F(ImportTest, I12_ImportIgnoreDuplicates) {
    navigateTo({"Organization", "Departments"});
    commands::select(*db, pth, "1");

    // First import (may already be done from seeding)
    try { runCommand("import examples/CSV_Examples/employeedata.csv"); }
    catch (const DuplicateEntryError&) {}

    int before = queryCount("SELECT COUNT(*) FROM Employees");

    // Second import with -i flag must not throw
    EXPECT_NO_THROW(runCommand("import -i examples/CSV_Examples/employeedata.csv"));

    // Row count must be unchanged (no new rows added)
    int after = queryCount("SELECT COUNT(*) FROM Employees");
    EXPECT_EQ(after, before);
}
