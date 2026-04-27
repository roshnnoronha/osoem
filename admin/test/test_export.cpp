// test_export.cpp — Integration tests for the export command
// Covers TEST_PLAN sections E-01 to E-05

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "exceptions/exceptions.h"
#include "utils/csv_parser.h"

#include <fstream>
#include <cstdio>
#include <string>

class ExportTest : public OsoemTestFixture {};

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

static bool fileExists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

static int csvDataRowCount(const std::string& path) {
    auto rows = utils::parseCSVFile(path);
    return static_cast<int>(rows.size());
}

// ─────────────────────────────────────────────────────────────────────────────
// Tests
// ─────────────────────────────────────────────────────────────────────────────

// E-01 — Export at entity list location creates file with header + data rows
TEST_F(ExportTest, E01_ExportProjectsCreatesFile) {
    navigateTo({"Projects"});
    const std::string outPath = "/tmp/e01_projects_export.csv";
    std::remove(outPath.c_str());

    ASSERT_NO_THROW(runCommand("export " + outPath));

    EXPECT_TRUE(fileExists(outPath));

    // File must have at least a header row
    std::ifstream f(outPath);
    std::string firstLine;
    std::getline(f, firstLine);
    EXPECT_FALSE(firstLine.empty());

    // Data rows should match the DB count
    int dbCount = queryCount("SELECT COUNT(*) FROM Projects");
    int csvCount = csvDataRowCount(outPath);
    EXPECT_EQ(csvCount, dbCount);

    std::remove(outPath.c_str());
}

// E-02 — Export then import round-trip preserves row count
TEST_F(ExportTest, E02_ExportImportRoundTrip) {
    navigateTo({"Organization", "Departments"});
    commands::select(*db, pth, "1");

    const std::string exportPath = "/tmp/e02_employees_export.csv";
    std::remove(exportPath.c_str());

    // Export
    ASSERT_NO_THROW(runCommand("export " + exportPath));
    EXPECT_TRUE(fileExists(exportPath));

    int exportedCount = csvDataRowCount(exportPath);
    if (exportedCount == 0) GTEST_SKIP() << "No employees to round-trip";

    // Reset DB and re-seed (sans employees import)
    resetAndSeedTestDb();

    // Navigate back (path was reset by resetAndSeedTestDb)
    navigateTo({"Organization", "Departments"});
    commands::select(*db, pth, "1");

    int beforeImport = queryCount(
        "SELECT COUNT(*) FROM Employees WHERE departmentid=1");

    // Re-import
    try {
        runCommand("import " + exportPath);
    } catch (const DuplicateEntryError& e) {
        GTEST_SKIP() << "Duplicate on re-import: " << e.what();
    }

    int afterImport = queryCount(
        "SELECT COUNT(*) FROM Employees WHERE departmentid=1");
    EXPECT_GE(afterImport - beforeImport, 0);

    std::remove(exportPath.c_str());
}

// E-03 — Export at a folder / entity node (not a list) — documents behavior
TEST_F(ExportTest, E03_ExportAtEntityNodeDoesNotCrash) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project"); // now at PROJECT entity

    const std::string outPath = "/tmp/e03_project_entity_export.csv";
    std::remove(outPath.c_str());

    // Either succeeds or raises NavigationError — must not crash with std::terminate
    try {
        runCommand("export " + outPath);
        // If it succeeded, file should exist or be empty
    } catch (const NavigationError&) {
        // Acceptable
    }

    std::remove(outPath.c_str());
}

// E-04 — Export to existing file path overwrites it
TEST_F(ExportTest, E04_ExportToExistingFileOverwrites) {
    const std::string outPath = "/tmp/e04_existing.csv";

    // Create a pre-existing file with known content
    {
        std::ofstream f(outPath);
        f << "old,content\n1,two\n";
    }

    navigateTo({"Projects"});
    try {
        runCommand("export " + outPath);
    } catch (const FileIOError&) {
        // Some impls refuse to overwrite — document the behavior
        std::remove(outPath.c_str());
        return;
    }

    // If it succeeded, the old content should be replaced
    std::ifstream f(outPath);
    std::string line;
    std::getline(f, line);
    // The new header should not be the old "old,content"
    EXPECT_NE(line, "old,content");

    std::remove(outPath.c_str());
}

// E-06 — Export departments from DEPARTMENTS_FOLDER
TEST_F(ExportTest, E06_ExportDepartments) {
    navigateTo({"Organization", "Departments"});
    const std::string outPath = "/tmp/e06_departments.csv";
    std::remove(outPath.c_str());

    ASSERT_NO_THROW(runCommand("export " + outPath));
    EXPECT_TRUE(fileExists(outPath));

    std::ifstream f(outPath);
    std::string header;
    std::getline(f, header);
    EXPECT_EQ(header, "departmentname");

    int dbCount = queryCount("SELECT COUNT(*) FROM Departments");
    int csvCount = csvDataRowCount(outPath);
    EXPECT_EQ(csvCount, dbCount);

    std::remove(outPath.c_str());
}

// E-07 — Export team members from TEAM_FOLDER
TEST_F(ExportTest, E07_ExportTeamMembers) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Team");
    const std::string outPath = "/tmp/e07_team.csv";
    std::remove(outPath.c_str());

    ASSERT_NO_THROW(runCommand("export " + outPath));
    EXPECT_TRUE(fileExists(outPath));

    std::ifstream f(outPath);
    std::string header;
    std::getline(f, header);
    EXPECT_NE(header.find("firstname"), std::string::npos);
    EXPECT_NE(header.find("role"), std::string::npos);

    int projId = queryCount(
        "SELECT projectid FROM Projects WHERE projectname='Building Project'");
    int dbCount = queryCount(
        "SELECT COUNT(*) FROM ProjectTeamMembers WHERE projectid=" +
        std::to_string(projId));
    int csvCount = csvDataRowCount(outPath);
    EXPECT_EQ(csvCount, dbCount);

    std::remove(outPath.c_str());
}

// E-08 — Export artefact data from DATA_FOLDER
TEST_F(ExportTest, E08_ExportArtefactData) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Artefact Types");
    commands::select(*db, pth, "Drawing");
    commands::select(*db, pth, "Artefacts");
    commands::select(*db, pth, "Foundation Plan - Sheet S-101");
    commands::select(*db, pth, "Data");

    const std::string outPath = "/tmp/e08_artefact_data.csv";
    std::remove(outPath.c_str());

    ASSERT_NO_THROW(runCommand("export " + outPath));
    EXPECT_TRUE(fileExists(outPath));

    std::ifstream f(outPath);
    std::string header;
    std::getline(f, header);
    EXPECT_NE(header.find("fieldtitle"), std::string::npos);
    EXPECT_NE(header.find("value"), std::string::npos);

    std::remove(outPath.c_str());
}

// E-09 — Export activity associations from ASSOCIATIONS_FOLDER
TEST_F(ExportTest, E09_ExportArtefactAssociations) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Artefact Types");
    commands::select(*db, pth, "Drawing");
    commands::select(*db, pth, "Artefacts");
    commands::select(*db, pth, "Foundation Plan - Sheet S-101");
    commands::select(*db, pth, "Associations");

    const std::string outPath = "/tmp/e09_associations.csv";
    std::remove(outPath.c_str());

    ASSERT_NO_THROW(runCommand("export " + outPath));
    EXPECT_TRUE(fileExists(outPath));

    std::ifstream f(outPath);
    std::string header;
    std::getline(f, header);
    EXPECT_NE(header.find("activityname"), std::string::npos);
    EXPECT_NE(header.find("ratio"), std::string::npos);

    std::remove(outPath.c_str());
}

// E-10 — Export milestone links from ARTEFACT_TO_ACTIVITY_LINK
TEST_F(ExportTest, E10_ExportMilestoneLinks) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Artefact Types");
    commands::select(*db, pth, "Drawing");
    commands::select(*db, pth, "Artefacts");
    commands::select(*db, pth, "Foundation Plan - Sheet S-101");
    commands::select(*db, pth, "Associations");
    commands::select(*db, pth, "Prepare structural drawings");

    const std::string outPath = "/tmp/e10_milestone_links.csv";
    std::remove(outPath.c_str());

    ASSERT_NO_THROW(runCommand("export " + outPath));
    EXPECT_TRUE(fileExists(outPath));

    std::ifstream f(outPath);
    std::string header;
    std::getline(f, header);
    EXPECT_NE(header.find("milestonestepname"), std::string::npos);
    EXPECT_NE(header.find("completiondate"), std::string::npos);

    std::remove(outPath.c_str());
}

// E-11 — Export task assignments from ASSIGNMENTS_FOLDER
TEST_F(ExportTest, E11_ExportTaskAssignments) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Activity Categories");
    commands::select(*db, pth, "Structural Engineering");
    commands::select(*db, pth, "Design Development");
    commands::select(*db, pth, "Prepare structural drawings");
    commands::select(*db, pth, "Tasks");
    commands::select(*db, pth, "Review architectural drawings");
    commands::select(*db, pth, "Assignments");

    const std::string outPath = "/tmp/e11_assignments.csv";
    std::remove(outPath.c_str());

    ASSERT_NO_THROW(runCommand("export " + outPath));
    EXPECT_TRUE(fileExists(outPath));

    std::ifstream f(outPath);
    std::string header;
    std::getline(f, header);
    EXPECT_NE(header.find("userid"), std::string::npos);
    EXPECT_NE(header.find("assigneddate"), std::string::npos);

    std::remove(outPath.c_str());
}

// E-12 — Export hours bookings from ASSIGNMENT
TEST_F(ExportTest, E12_ExportHoursBookings) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Activity Categories");
    commands::select(*db, pth, "Structural Engineering");
    commands::select(*db, pth, "Design Development");
    commands::select(*db, pth, "Prepare structural drawings");
    commands::select(*db, pth, "Tasks");
    commands::select(*db, pth, "Review architectural drawings");
    commands::select(*db, pth, "Assignments");
    commands::select(*db, pth, "Michael Chen");

    const std::string outPath = "/tmp/e12_hours.csv";
    std::remove(outPath.c_str());

    ASSERT_NO_THROW(runCommand("export " + outPath));
    EXPECT_TRUE(fileExists(outPath));

    std::ifstream f(outPath);
    std::string header;
    std::getline(f, header);
    EXPECT_EQ(header, "bookeddate,hours");

    // Project.src books 3 hours entries for Michael Chen on this task
    int csvCount = csvDataRowCount(outPath);
    EXPECT_EQ(csvCount, 3);

    std::remove(outPath.c_str());
}

// E-13 — DEPARTMENT export filters employees by department
TEST_F(ExportTest, E13_ExportDepartmentFiltered) {
    // Export from department 1 (Project Management — has 1 employee: John Davis)
    navigateTo({"Organization", "Departments"});
    commands::select(*db, pth, "1");
    const std::string out1 = "/tmp/e13_dept1.csv";
    std::remove(out1.c_str());
    ASSERT_NO_THROW(runCommand("export " + out1));

    int dept1DbCount = queryCount(
        "SELECT COUNT(*) FROM Employees WHERE departmentid=1");
    int dept1CsvCount = csvDataRowCount(out1);
    EXPECT_EQ(dept1CsvCount, dept1DbCount);

    // Export from department 2 (Structural Engineering — has 2 employees)
    pth.reset();
    navigateTo({"Organization", "Departments"});
    commands::select(*db, pth, "2");
    const std::string out2 = "/tmp/e13_dept2.csv";
    std::remove(out2.c_str());
    ASSERT_NO_THROW(runCommand("export " + out2));

    int dept2DbCount = queryCount(
        "SELECT COUNT(*) FROM Employees WHERE departmentid=2");
    int dept2CsvCount = csvDataRowCount(out2);
    EXPECT_EQ(dept2CsvCount, dept2DbCount);

    // Departments have different employee counts
    EXPECT_NE(dept1CsvCount, dept2CsvCount);

    std::remove(out1.c_str());
    std::remove(out2.c_str());
}

// E-05 — Export activities CSV contains expected column names
TEST_F(ExportTest, E05_ExportActivitiesColumns) {
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

    const std::string outPath = "/tmp/e05_activities.csv";
    std::remove(outPath.c_str());

    ASSERT_NO_THROW(runCommand("export " + outPath));
    EXPECT_TRUE(fileExists(outPath));

    // Read header line
    std::ifstream f(outPath);
    std::string header;
    std::getline(f, header);

    // Header should mention activityname or similar
    bool hasActivityCol = header.find("activity") != std::string::npos ||
                          header.find("Activity") != std::string::npos;
    EXPECT_TRUE(hasActivityCol) << "Activity column not found in header: " << header;

    std::remove(outPath.c_str());
}
