// test_remove.cpp — Integration tests for the remove command
// Covers TEST_PLAN sections R-01 to R-08

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "exceptions/exceptions.h"

class RemoveTest : public OsoemTestFixture {
protected:
    // Helper: ensures a project named `name` exists
    void ensureProject(const std::string& name, const std::string& no) {
        navigateTo({"Projects"});
        try {
            runCommand("add \"projectname:" + name + ",projectno:" + no + "\"");
        } catch (const DuplicateEntryError&) {}
        pth.reset();
    }
};

// R-01 — Remove by name
TEST_F(RemoveTest, R01_RemoveByName) {
    ensureProject("R01 Test Project", "R01-001");
    navigateTo({"Projects"});
    ASSERT_NO_THROW(runCommand("remove \"R01 Test Project\""));

    int count = queryCount(
        "SELECT COUNT(*) FROM Projects WHERE projectname='R01 Test Project'");
    EXPECT_EQ(count, 0);
}

// R-02 — Remove by numeric ID
TEST_F(RemoveTest, R02_RemoveById) {
    ensureProject("R02 Test Project", "R02-001");
    navigateTo({"Projects"});

    int projId = queryCount(
        "SELECT projectid FROM Projects WHERE projectname='R02 Test Project'");
    ASSERT_GT(projId, 0);

    ASSERT_NO_THROW(runCommand("remove " + std::to_string(projId)));

    int count = queryCount(
        "SELECT COUNT(*) FROM Projects WHERE projectid=" + std::to_string(projId));
    EXPECT_EQ(count, 0);
}

// R-03 — Remove non-existent item raises EntityNotFoundError
TEST_F(RemoveTest, R03_RemoveNonExistentItem) {
    navigateTo({"Projects"});
    EXPECT_THROW(runCommand("remove \"Phantom Project ZZZZ\""), EntityNotFoundError);
    // Path must remain at Projects
    EXPECT_EQ(pth.current_type(), data::Path::PROJECTS_FOLDER);
}

// R-04 — Remove with no argument raises NavigationError
TEST_F(RemoveTest, R04_RemoveWithNoArgument) {
    navigateTo({"Projects"});
    EXPECT_THROW(runCommand("remove"), NavigationError);
}

// R-06 — Remove project team member (employee still exists)
TEST_F(RemoveTest, R06_RemoveTeamMember) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Team");

    // Add a member to ensure one exists
    try {
        runCommand("add \"employeename:Christopher Lee,role:1\"");
    } catch (const DuplicateEntryError&) {}

    int membersBefore = queryCount("SELECT COUNT(*) FROM ProjectTeamMembers");
    int empBefore = queryCount("SELECT COUNT(*) FROM Employees");

    ASSERT_NO_THROW(runCommand("remove \"Christopher Lee\""));

    int membersAfter = queryCount("SELECT COUNT(*) FROM ProjectTeamMembers");
    int empAfter = queryCount("SELECT COUNT(*) FROM Employees");

    // Team membership removed
    EXPECT_LT(membersAfter, membersBefore);
    // Employee record untouched
    EXPECT_EQ(empAfter, empBefore);
}

// R-07 — Remove activity category raises DatabaseError or cascades
TEST_F(RemoveTest, R07_RemoveCategoryWithSubcategories) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Activity Categories");

    int catId = queryCount(
        "SELECT categoryid FROM ActivityCategories "
        "WHERE projectid = (SELECT projectid FROM Projects WHERE projectname='Building Project') "
        "LIMIT 1");
    if (catId == 0) GTEST_SKIP() << "No categories to remove";

    std::string catName = queryString(
        "SELECT categoryname FROM ActivityCategories WHERE categoryid=" +
        std::to_string(catId));

    // Expect either a cascade delete or a FK constraint error — either is valid
    try {
        runCommand("remove \"" + catName + "\"");
        // If it succeeded, category should be gone
        int count = queryCount(
            "SELECT COUNT(*) FROM ActivityCategories WHERE categoryid=" +
            std::to_string(catId));
        EXPECT_EQ(count, 0);
    } catch (const DatabaseError&) {
        // FK constraint prevented removal — also valid
    } catch (const ConstraintViolationError&) {
        // Also valid
    }
}

// R-08 — Remove employee referenced as activity manager raises DatabaseError
TEST_F(RemoveTest, R08_RemoveEmployeeUsedAsManager) {
    // Find an employee who is an activity manager
    int empId = queryCount(
        "SELECT DISTINCT activitymanagerid FROM Activities LIMIT 1");
    if (empId == 0) GTEST_SKIP() << "No activities with managers";

    std::string deptId = queryString(
        "SELECT departmentid FROM Employees WHERE employeeid=" +
        std::to_string(empId));
    std::string empName = queryString(
        "SELECT CONCAT(firstname,' ',lastname) FROM Employees WHERE employeeid=" +
        std::to_string(empId));

    navigateTo({"Organization", "Departments"});
    commands::select(*db, pth, deptId);

    EXPECT_THROW(runCommand("remove \"" + empName + "\""), DatabaseError);
}
