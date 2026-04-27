// test_list.cpp — Integration tests for the list command
// Covers TEST_PLAN sections L-01 to L-07

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "exceptions/exceptions.h"
#include "utils/csv_parser.h"

class ListTest : public OsoemTestFixture {};

// L-01 — List at root shows Organization and Projects
TEST_F(ListTest, L01_ListAtRoot) {
    std::string output = runCommand("list");
    EXPECT_NE(output.find("Organization"), std::string::npos);
    EXPECT_NE(output.find("Projects"), std::string::npos);
}

// Alias: ls works the same as list
TEST_F(ListTest, L01_LsAliasAtRoot) {
    std::string out1 = runCommand("list");
    pth.reset();
    std::string out2 = runCommand("ls");
    EXPECT_EQ(out1, out2);
}

// L-02 — Default list format shows ID and name columns
TEST_F(ListTest, L02_DefaultListAtProjects) {
    navigateTo({"Projects"});
    std::string output = runCommand("ls");
    // Should contain at least the project seeded by project.src
    EXPECT_FALSE(output.empty());
    // "Building Project" or at minimum a numeric ID should appear
    EXPECT_NE(output.find("Building"), std::string::npos);
}

// L-03 — Detailed list with -a flag shows extra columns
TEST_F(ListTest, L03_DetailedListWithFlagA) {
    // Navigate deep enough to list activities
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Activity Categories");

    // Pick first category
    int catId = queryCount(
        "SELECT categoryid FROM ActivityCategories "
        "WHERE projectid = (SELECT projectid FROM Projects WHERE projectname='Building Project') "
        "LIMIT 1");
    if (catId == 0) GTEST_SKIP() << "No categories in test DB";

    commands::select(*db, pth, std::to_string(catId));

    // Pick first subcategory
    int subId = queryCount(
        "SELECT subcategoryid FROM ActivitySubcategories "
        "WHERE categoryid = " + std::to_string(catId) + " LIMIT 1");
    if (subId == 0) GTEST_SKIP() << "No subcategories";

    commands::select(*db, pth, std::to_string(subId));

    std::string output = runCommand("ls -a");
    // Detailed list should include date or hours columns
    bool hasExtra = output.find("planned") != std::string::npos ||
                    output.find("hours")   != std::string::npos ||
                    output.find("manager") != std::string::npos;
    EXPECT_TRUE(hasExtra) << "Expected extra columns in -a output. Got:\n" << output;
}

// L-04 — List empty folder produces no crash
TEST_F(ListTest, L04_ListEmptyMilestonesFolder) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Milestones");

    // Regardless of whether milestones exist, this must not throw
    EXPECT_NO_THROW(runCommand("ls"));
}

// L-05 — List after add reflects new item
TEST_F(ListTest, L05_ListAfterAddShowsNewItem) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Milestones");

    // Count items before
    std::string before = runCommand("ls");

    // Add a milestone
    ASSERT_NO_THROW(runCommand("add \"milestonename:L05 Test Milestone\""));

    std::string after = runCommand("ls");
    EXPECT_NE(after.find("L05 Test Milestone"), std::string::npos);
}

// L-06 — List after remove shows item absent
TEST_F(ListTest, L06_ListAfterRemoveShowsItemAbsent) {
    // Depends on L-05 having run first (or re-adds), but we add our own item.
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Milestones");

    // Ensure the item exists
    runCommand("add \"milestonename:L06 To Remove\"");
    ASSERT_NE(runCommand("ls").find("L06 To Remove"), std::string::npos);

    // Remove it
    ASSERT_NO_THROW(runCommand("remove \"L06 To Remove\""));

    std::string after = runCommand("ls");
    EXPECT_EQ(after.find("L06 To Remove"), std::string::npos);
}

// L-07 — List count after CSV import matches file row count
TEST_F(ListTest, L07_ListCountMatchesImportCount) {
    navigateTo({"Projects"});

    // Count rows in projectdata.csv (excluding header)
    auto rows = utils::parseCSVFile("examples/CSV_Examples/projectdata.csv");
    int csvCount = static_cast<int>(rows.size());
    if (csvCount == 0) GTEST_SKIP() << "projectdata.csv is empty";

    // Count existing projects before import
    int beforeCount = queryCount("SELECT COUNT(*) FROM Projects");

    // Import — may throw DuplicateEntryError if already imported; that's fine
    try {
        runCommand("import examples/CSV_Examples/projectdata.csv");
    } catch (const DuplicateEntryError&) {
        GTEST_SKIP() << "Projects already imported; skipping count check";
    }

    int afterCount = queryCount("SELECT COUNT(*) FROM Projects");
    EXPECT_EQ(afterCount - beforeCount, csvCount);
}
