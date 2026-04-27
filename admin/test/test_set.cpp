// test_set.cpp — Integration tests for the set command
// Covers TEST_PLAN sections S-01 to S-10

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "exceptions/exceptions.h"

class SetTest : public OsoemTestFixture {
protected:
    // Helper: navigate into the first project
    void navToFirstProject() {
        navigateTo({"Projects"});
        int projId = queryCount("SELECT projectid FROM Projects LIMIT 1");
        if (projId == 0) ADD_FAILURE() << "No projects in test DB";
        commands::select(*db, pth, std::to_string(projId));
    }

    // Helper: navigate into the first activity
    bool navToFirstActivity() {
        navigateTo({"Projects"});
        int projId = queryCount("SELECT projectid FROM Projects LIMIT 1");
        if (projId == 0) return false;
        commands::select(*db, pth, std::to_string(projId));
        commands::select(*db, pth, "Activity Categories");

        int catId = queryCount(
            "SELECT categoryid FROM ActivityCategories WHERE projectid=" +
            std::to_string(projId) + " LIMIT 1");
        if (catId == 0) return false;
        commands::select(*db, pth, std::to_string(catId));

        int subId = queryCount(
            "SELECT subcategoryid FROM ActivitySubcategories WHERE categoryid=" +
            std::to_string(catId) + " LIMIT 1");
        if (subId == 0) return false;
        commands::select(*db, pth, std::to_string(subId));

        int actId = queryCount(
            "SELECT activityid FROM Activities WHERE subcategoryid=" +
            std::to_string(subId) + " LIMIT 1");
        if (actId == 0) return false;
        commands::select(*db, pth, std::to_string(actId));
        return true;
    }
};

// S-01 — Set valid text field on project entity
TEST_F(SetTest, S01_SetProjectName) {
    navToFirstProject();
    EXPECT_NO_THROW(runCommand("set projectname \"S01 Updated Project\""));

    // Verify update persisted
    std::string name = queryString(
        "SELECT projectname FROM Projects WHERE projectid=" +
        std::to_string(pth.current_id()));
    EXPECT_EQ(name, "S01 Updated Project");

    // Reset name to avoid breaking other tests
    runCommand("set projectname \"Building Project\"");
}

// S-02 — Set date field (valid ISO format)
TEST_F(SetTest, S02_SetActivityDateValid) {
    if (!navToFirstActivity()) GTEST_SKIP() << "No activities";
    EXPECT_NO_THROW(runCommand("set plannedstart 2026-05-01"));

    std::string date = queryString(
        "SELECT DATE_FORMAT(plannedstart,'%Y-%m-%d') FROM Activities "
        "WHERE activityid=" + std::to_string(pth.current_id()));
    EXPECT_EQ(date, "2026-05-01");
}

// S-03 — Set date field with invalid format raises ValidationError
TEST_F(SetTest, S03_SetActivityDateInvalidFormat) {
    if (!navToFirstActivity()) GTEST_SKIP() << "No activities";
    EXPECT_THROW(runCommand("set plannedstart 01/05/2026"), ValidationError);
}

// S-04 — Set unknown field name raises InvalidFieldError
TEST_F(SetTest, S04_SetUnknownFieldName) {
    navToFirstProject();
    EXPECT_THROW(runCommand("set nonexistentfield somevalue"), InvalidFieldError);
}

// S-05 — Set numeric field to non-numeric value raises ValidationError
TEST_F(SetTest, S05_SetNumericFieldToNonNumeric) {
    if (!navToFirstActivity()) GTEST_SKIP() << "No activities";
    EXPECT_THROW(runCommand("set plannedhours notanumber"), ValidationError);
}

// S-06 — Set field at a folder node raises NavigationError
TEST_F(SetTest, S06_SetAtFolderNode) {
    navigateTo({"Projects"});
    // PROJECTS_FOLDER is a folder node — cannot set
    EXPECT_THROW(runCommand("set name X"), NavigationError);
}

// S-07 — Set field at ROOT raises NavigationError
TEST_F(SetTest, S07_SetAtRoot) {
    // pth starts at ROOT
    EXPECT_THROW(runCommand("set name test"), NavigationError);
}

// S-08 — Set employee password stores new hash
TEST_F(SetTest, S08_SetEmployeePassword) {
    navigateTo({"Organization", "Departments"});
    commands::select(*db, pth, "1");

    int empId = queryCount(
        "SELECT employeeid FROM Employees WHERE departmentid=1 LIMIT 1");
    if (empId == 0) GTEST_SKIP() << "No employees in dept 1";

    commands::select(*db, pth, std::to_string(empId));

    std::string oldHash = queryString(
        "SELECT password FROM Employees WHERE employeeid=" +
        std::to_string(empId));

    EXPECT_NO_THROW(runCommand("set password newpassword123"));

    std::string newHash = queryString(
        "SELECT password FROM Employees WHERE employeeid=" +
        std::to_string(empId));
    std::string newSalt = queryString(
        "SELECT passwordsalt FROM Employees WHERE employeeid=" +
        std::to_string(empId));

    EXPECT_EQ(newHash.size(), 64u);
    EXPECT_EQ(newSalt.size(), 64u);
    // Hash should have changed
    EXPECT_NE(newHash, oldHash);
    // Must not contain plaintext
    EXPECT_EQ(newHash.find("newpassword123"), std::string::npos);
}

// S-09 — Set milestone step progressratio
TEST_F(SetTest, S09_SetMilestoneStepProgressRatio) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Milestones");

    int milId = queryCount(
        "SELECT milestoneid FROM Milestones "
        "WHERE projectid = (SELECT projectid FROM Projects WHERE projectname='Building Project') "
        "LIMIT 1");
    if (milId == 0) GTEST_SKIP() << "No milestones";

    commands::select(*db, pth, std::to_string(milId));

    int stepId = queryCount(
        "SELECT milestonestepid FROM MilestoneSteps "
        "WHERE milestoneid=" + std::to_string(milId) + " LIMIT 1");
    if (stepId == 0) GTEST_SKIP() << "No milestone steps";

    commands::select(*db, pth, std::to_string(stepId));

    EXPECT_NO_THROW(runCommand("set progressratio 0.75"));

    std::string val = queryString(
        "SELECT progressratio FROM MilestoneSteps WHERE milestonestepid=" +
        std::to_string(stepId));
    EXPECT_NEAR(std::stod(val), 0.75, 0.001);
}
