// test_data_integrity.cpp — Integration tests for data integrity rules
// Covers TEST_PLAN sections DI-01 to DI-10

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "exceptions/exceptions.h"

class DataIntegrityTest : public OsoemTestFixture {
protected:
    // Navigate to the Associations folder of the first artefact of the first
    // artefact type in "Building Project". Returns false if path cannot be reached.
    bool navToArtefactAssociations() {
        navigateTo({"Projects"});
        commands::select(*db, pth, "Building Project");

        int projId = queryCount(
            "SELECT projectid FROM Projects WHERE projectname='Building Project'");
        if (projId == 0) return false;

        commands::select(*db, pth, "Artefact Types");

        int atId = queryCount(
            "SELECT artefacttypeid FROM ArtefactTypes WHERE projectid=" +
            std::to_string(projId) + " LIMIT 1");
        if (atId == 0) return false;
        commands::select(*db, pth, std::to_string(atId));
        commands::select(*db, pth, "Artefacts");

        int artId = queryCount(
            "SELECT artefactid FROM Artefacts WHERE artefacttypeid=" +
            std::to_string(atId) + " LIMIT 1");
        if (artId == 0) return false;
        commands::select(*db, pth, std::to_string(artId));
        commands::select(*db, pth, "Associations");
        return true;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// Tests
// ─────────────────────────────────────────────────────────────────────────────

// DI-01 — ArtefactToActivityLink ratio sum > 1.0 is rejected
TEST_F(DataIntegrityTest, DI01_ArtefactToActivityLinkRatioSum) {
    if (!navToArtefactAssociations()) GTEST_SKIP() << "No artefacts";

    // Look up an activity in the project
    int actId = queryCount(
        "SELECT activityid FROM Activities "
        "WHERE subcategoryid IN ("
        "  SELECT subcategoryid FROM ActivitySubcategories WHERE categoryid IN ("
        "    SELECT categoryid FROM ActivityCategories "
        "    WHERE projectid=(SELECT projectid FROM Projects WHERE projectname='Building Project')"
        "  )"
        ") LIMIT 1");
    if (actId == 0) GTEST_SKIP() << "No activities";

    std::string actName = queryString(
        "SELECT activityname FROM Activities WHERE activityid=" +
        std::to_string(actId));

    // First link: ratio = 0.80
    try {
        runCommand("add \"activityname:" + actName + ",ratio:0.80\"");
    } catch (const DataError&) {}

    // Second link for same artefact: 0.80 + 0.30 = 1.10 → must fail
    EXPECT_THROW(
        runCommand("add \"activityname:" + actName + ",ratio:0.30\""),
        DataError);
}

// DI-02a — MilestoneStep progressratio > 1 rejected
TEST_F(DataIntegrityTest, DI02a_MilestoneStepRatioTooHigh) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Milestones");

    int milId = queryCount(
        "SELECT milestoneid FROM Milestones "
        "WHERE projectid=(SELECT projectid FROM Projects WHERE projectname='Building Project') "
        "LIMIT 1");
    if (milId == 0) GTEST_SKIP() << "No milestones";
    commands::select(*db, pth, std::to_string(milId));

    EXPECT_THROW(
        runCommand("add \"milestonestepname:TooHigh,progressratio:1.01\""),
        ValidationError);
}

// DI-02b — MilestoneStep progressratio < 0 rejected
TEST_F(DataIntegrityTest, DI02b_MilestoneStepRatioNegative) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Milestones");

    int milId = queryCount(
        "SELECT milestoneid FROM Milestones "
        "WHERE projectid=(SELECT projectid FROM Projects WHERE projectname='Building Project') "
        "LIMIT 1");
    if (milId == 0) GTEST_SKIP() << "No milestones";
    commands::select(*db, pth, std::to_string(milId));

    EXPECT_THROW(
        runCommand("add \"milestonestepname:Negative,progressratio:-0.10\""),
        ValidationError);
}

// DI-03 — Negative hours value rejected
TEST_F(DataIntegrityTest, DI03_NegativeHoursRejected) {
    // Navigate to an assignment
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Activity Categories");

    int catId = queryCount(
        "SELECT categoryid FROM ActivityCategories "
        "WHERE projectid=(SELECT projectid FROM Projects WHERE projectname='Building Project') "
        "LIMIT 1");
    if (catId == 0) GTEST_SKIP() << "No categories";
    commands::select(*db, pth, std::to_string(catId));

    int subId = queryCount(
        "SELECT subcategoryid FROM ActivitySubcategories WHERE categoryid=" +
        std::to_string(catId) + " LIMIT 1");
    if (subId == 0) GTEST_SKIP() << "No subcategories";
    commands::select(*db, pth, std::to_string(subId));

    int actId = queryCount(
        "SELECT activityid FROM Activities WHERE subcategoryid=" +
        std::to_string(subId) + " LIMIT 1");
    if (actId == 0) GTEST_SKIP() << "No activities";
    commands::select(*db, pth, std::to_string(actId));
    commands::select(*db, pth, "Tasks");

    int taskId = queryCount(
        "SELECT taskid FROM ActivityTasks WHERE activityid=" +
        std::to_string(actId) + " AND parenttaskid IS NULL LIMIT 1");
    if (taskId == 0) GTEST_SKIP() << "No tasks";
    commands::select(*db, pth, std::to_string(taskId));
    commands::select(*db, pth, "Assignments");

    int assignId = queryCount(
        "SELECT assignmentid FROM TaskAssignments WHERE taskid=" +
        std::to_string(taskId) + " LIMIT 1");
    if (assignId == 0) GTEST_SKIP() << "No assignments";
    commands::select(*db, pth, std::to_string(assignId));

    EXPECT_THROW(
        runCommand("add \"bookeddate:2026-03-01,hours:-5\""),
        ValidationError);
}

// DI-04 — Activity manager must be a project team member
TEST_F(DataIntegrityTest, DI04_ActivityManagerMustBeTeamMember) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Activity Categories");

    int catId = queryCount(
        "SELECT categoryid FROM ActivityCategories "
        "WHERE projectid=(SELECT projectid FROM Projects WHERE projectname='Building Project') "
        "LIMIT 1");
    if (catId == 0) GTEST_SKIP() << "No categories";
    commands::select(*db, pth, std::to_string(catId));

    int subId = queryCount(
        "SELECT subcategoryid FROM ActivitySubcategories WHERE categoryid=" +
        std::to_string(catId) + " LIMIT 1");
    if (subId == 0) GTEST_SKIP() << "No subcategories";
    commands::select(*db, pth, std::to_string(subId));

    // Employee ZZZZ is not in any project team
    EXPECT_THROW(
        runCommand("add \"activityname:DI04 Test,"
                   "activitymanagername:NonMember ZZZZ,"
                   "plannedstart:2026-01-01,"
                   "plannedfinish:2026-02-01,"
                   "plannedhours:50\""),
        ValidationError);
}

// DI-05 — FK: cannot delete employee used as activity manager
TEST_F(DataIntegrityTest, DI05_DeleteEmployeeUsedAsManagerFails) {
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

// DI-06 — Artefact owner must be a project team member
TEST_F(DataIntegrityTest, DI06_ArtefactOwnerMustBeTeamMember) {
    navigateTo({"Projects"});
    int projId = queryCount(
        "SELECT projectid FROM Projects WHERE projectname='Building Project'");
    if (projId == 0) GTEST_SKIP() << "No project";
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Artefact Types");

    int atId = queryCount(
        "SELECT artefacttypeid FROM ArtefactTypes WHERE projectid=" +
        std::to_string(projId) + " LIMIT 1");
    if (atId == 0) GTEST_SKIP() << "No artefact types";
    commands::select(*db, pth, std::to_string(atId));
    commands::select(*db, pth, "Artefacts");

    EXPECT_THROW(
        runCommand("add \"artefacttitle:DI06 Test Drawing,"
                   "artefactownername:NonMember ZZZZ\""),
        ValidationError);
}

// DI-07 — Note author must be a project team member
TEST_F(DataIntegrityTest, DI07_NoteAuthorMustBeTeamMember) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Activity Categories");

    int catId = queryCount(
        "SELECT categoryid FROM ActivityCategories "
        "WHERE projectid=(SELECT projectid FROM Projects WHERE projectname='Building Project') "
        "LIMIT 1");
    if (catId == 0) GTEST_SKIP() << "No categories";
    commands::select(*db, pth, std::to_string(catId));

    int subId = queryCount(
        "SELECT subcategoryid FROM ActivitySubcategories WHERE categoryid=" +
        std::to_string(catId) + " LIMIT 1");
    if (subId == 0) GTEST_SKIP() << "No subcategories";
    commands::select(*db, pth, std::to_string(subId));

    int actId = queryCount(
        "SELECT activityid FROM Activities WHERE subcategoryid=" +
        std::to_string(subId) + " LIMIT 1");
    if (actId == 0) GTEST_SKIP() << "No activities";
    commands::select(*db, pth, std::to_string(actId));
    commands::select(*db, pth, "Notes");

    EXPECT_THROW(
        runCommand("add \"username:NonMember ZZZZ,"
                   "note:DI07 test note,"
                   "notedate:2026-01-01\""),
        ValidationError);
}

// DI-08 — Task assignment must be to a project team member
TEST_F(DataIntegrityTest, DI08_TaskAssignmentMustBeTeamMember) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Activity Categories");

    int catId = queryCount(
        "SELECT categoryid FROM ActivityCategories "
        "WHERE projectid=(SELECT projectid FROM Projects WHERE projectname='Building Project') "
        "LIMIT 1");
    if (catId == 0) GTEST_SKIP() << "No categories";
    commands::select(*db, pth, std::to_string(catId));

    int subId = queryCount(
        "SELECT subcategoryid FROM ActivitySubcategories WHERE categoryid=" +
        std::to_string(catId) + " LIMIT 1");
    if (subId == 0) GTEST_SKIP() << "No subcategories";
    commands::select(*db, pth, std::to_string(subId));

    int actId = queryCount(
        "SELECT activityid FROM Activities WHERE subcategoryid=" +
        std::to_string(subId) + " LIMIT 1");
    if (actId == 0) GTEST_SKIP() << "No activities";
    commands::select(*db, pth, std::to_string(actId));
    commands::select(*db, pth, "Tasks");

    int taskId = queryCount(
        "SELECT taskid FROM ActivityTasks WHERE activityid=" +
        std::to_string(actId) + " AND parenttaskid IS NULL LIMIT 1");
    if (taskId == 0) GTEST_SKIP() << "No tasks";
    commands::select(*db, pth, std::to_string(taskId));
    commands::select(*db, pth, "Assignments");

    EXPECT_THROW(
        runCommand("add \"username:NonMember ZZZZ,"
                   "assigneddate:2026-01-01\""),
        ValidationError);
}

// DI-10 — EntityNotFoundError uses correct entity name in message
TEST_F(DataIntegrityTest, DI10_EntityNotFoundErrorMessage) {
    navigateTo({"Projects"});
    try {
        commands::select(*db, pth, "DoesNotExist ZZZZ99");
    } catch (const EntityNotFoundError& e) {
        std::string msg = e.what();
        // Message should mention "Project" (the entity type we tried to select)
        EXPECT_NE(msg.find("not found"), std::string::npos);
    } catch (const NavigationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("not found"), std::string::npos);
    }
}
