// test_add.cpp — Integration tests for the add command
// Covers TEST_PLAN sections A-01 to A-39

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "exceptions/exceptions.h"

class AddTest : public OsoemTestFixture {};

// ─────────────────────────────────────────────────────────────────────────────
// General add tests
// ─────────────────────────────────────────────────────────────────────────────

// A-03 — Add duplicate project name raises DuplicateEntryError
TEST_F(AddTest, A03_AddDuplicateProjectName) {
    navigateTo({"Projects"});
    // First add succeeds (or already exists from seed)
    try { runCommand("add \"projectname:A03 Test Project,projectno:A03-001\""); }
    catch (const DuplicateEntryError&) {}

    // Second add with same name must throw
    EXPECT_THROW(
        runCommand("add \"projectname:A03 Test Project,projectno:A03-002\""),
        DuplicateEntryError);
}

// A-04 — Add activity with invalid date format raises ValidationError
TEST_F(AddTest, A04_AddActivityWithInvalidDateFormat) {
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
        "WHERE categoryid = " + std::to_string(catId) + " LIMIT 1");
    if (subId == 0) GTEST_SKIP() << "No subcategories";
    commands::select(*db, pth, std::to_string(subId));

    // Use slash-separated date which should be invalid
    EXPECT_THROW(
        runCommand("add \"activityname:BadDate Activity,"
                   "activitymanagername:John Davis,"
                   "plannedstart:2026/01/01,"
                   "plannedfinish:2026-04-01,"
                   "plannedhours:100\""),
        ValidationError);
}

// A-05 — Add project with missing required field raises ValidationError
TEST_F(AddTest, A05_AddProjectMissingRequiredField) {
    navigateTo({"Projects"});
    EXPECT_THROW(
        runCommand("add \"projectno:A05-MISSING\""),
        ValidationError);
}

// A-06 — Add project with unknown field key raises error
TEST_F(AddTest, A06_AddProjectUnknownFieldKey) {
    navigateTo({"Projects"});
    EXPECT_THROW(
        runCommand("add \"projectname:A06 Test,unknownfield:abc\""),
        DataError); // InvalidFieldError or ValidationError are both DataError
}

// A-37 — Add at ROOT raises NavigationError
TEST_F(AddTest, A37_AddAtRootRaisesNavigationError) {
    // pth is at ROOT by default after SetUp
    EXPECT_THROW(runCommand("add \"name:test\""), NavigationError);
}

// ─────────────────────────────────────────────────────────────────────────────
// Entity-specific add tests
// ─────────────────────────────────────────────────────────────────────────────

// A-10 — Add Department
TEST_F(AddTest, A10_AddDepartment) {
    navigateTo({"Organization", "Departments"});

    try {
        db->executeUpdate("DELETE FROM Departments WHERE departmentname='A10 Structural'");
    } catch (...) {}

    int before = queryCount("SELECT COUNT(*) FROM Departments");
    ASSERT_NO_THROW(runCommand("add \"departmentname:A10 Structural\""));
    int after = queryCount("SELECT COUNT(*) FROM Departments");
    EXPECT_EQ(after, before + 1);
}

// A-11 — Add Employee stores hashed password
TEST_F(AddTest, A11_AddEmployeeHashedPassword) {
    navigateTo({"Organization", "Departments"});
    commands::select(*db, pth, "1"); // first department

    const std::string email = "a11.jane@osoem.test";
    try {
        db->executeUpdate("DELETE FROM Employees WHERE email='" + email + "'");
    } catch (...) {}

    ASSERT_NO_THROW(runCommand(
        "add \"firstname:A11Jane,lastname:Smith,email:" + email +
        ",password:securepass123\""));

    std::string hash = queryString(
        "SELECT password FROM Employees WHERE email='" + email + "'");
    std::string salt = queryString(
        "SELECT passwordsalt FROM Employees WHERE email='" + email + "'");

    EXPECT_EQ(hash.size(), 64u);
    EXPECT_EQ(salt.size(), 64u);
    EXPECT_EQ(hash.find("securepass123"), std::string::npos);
}

// A-12 — Add Employee with duplicate email raises DatabaseError
TEST_F(AddTest, A12_AddEmployeeDuplicateEmail) {
    navigateTo({"Organization", "Departments"});
    commands::select(*db, pth, "1");

    const std::string email = "a12.dup@osoem.test";
    try {
        db->executeUpdate("DELETE FROM Employees WHERE email='" + email + "'");
    } catch (...) {}
    runCommand("add \"firstname:A12,lastname:First,email:" + email + ",password:p1\"");

    EXPECT_THROW(
        runCommand("add \"firstname:A12,lastname:Second,email:" + email + ",password:p2\""),
        DatabaseError);
}

// A-13 — Add Project
TEST_F(AddTest, A13_AddProject) {
    navigateTo({"Projects"});

    try {
        db->executeUpdate("DELETE FROM Projects WHERE projectname='A13 Bridge Project'");
    } catch (...) {}

    int before = queryCount("SELECT COUNT(*) FROM Projects");
    ASSERT_NO_THROW(runCommand("add \"projectname:A13 Bridge Project,projectno:A13-2026\""));
    int after = queryCount("SELECT COUNT(*) FROM Projects");
    EXPECT_EQ(after, before + 1);
}

// A-14 — Add Activity Category
TEST_F(AddTest, A14_AddActivityCategory) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Activity Categories");

    try {
        db->executeUpdate("DELETE FROM ActivityCategories WHERE categoryname='A14 Civil'");
    } catch (...) {}

    int before = queryCount("SELECT COUNT(*) FROM ActivityCategories");
    ASSERT_NO_THROW(runCommand("add \"categoryname:A14 Civil\""));
    int after = queryCount("SELECT COUNT(*) FROM ActivityCategories");
    EXPECT_EQ(after, before + 1);
}

// A-15 — Add Subcategory
TEST_F(AddTest, A15_AddSubcategory) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Activity Categories");

    int catId = queryCount(
        "SELECT categoryid FROM ActivityCategories "
        "WHERE projectid = (SELECT projectid FROM Projects WHERE projectname='Building Project') "
        "LIMIT 1");
    if (catId == 0) GTEST_SKIP() << "No categories";
    commands::select(*db, pth, std::to_string(catId));

    try {
        db->executeUpdate("DELETE FROM ActivitySubcategories WHERE subcategoryname='A15 Foundation'");
    } catch (...) {}

    int before = queryCount("SELECT COUNT(*) FROM ActivitySubcategories");
    ASSERT_NO_THROW(runCommand("add \"subcategoryname:A15 Foundation\""));
    int after = queryCount("SELECT COUNT(*) FROM ActivitySubcategories");
    EXPECT_EQ(after, before + 1);
}

// A-17 — Add Activity with non-team-member manager raises ValidationError
TEST_F(AddTest, A17_AddActivityNonTeamMemberManager) {
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
        "WHERE categoryid = " + std::to_string(catId) + " LIMIT 1");
    if (subId == 0) GTEST_SKIP() << "No subcategories";
    commands::select(*db, pth, std::to_string(subId));

    EXPECT_THROW(
        runCommand("add \"activityname:A17 Test,"
                   "activitymanagername:NonExistentMember ZZZZ,"
                   "plannedstart:2026-03-01,"
                   "plannedfinish:2026-06-30,"
                   "plannedhours:100\""),
        ValidationError);
}

// A-25 — Add Artefact Type
TEST_F(AddTest, A25_AddArtefactType) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Artefact Types");

    try {
        db->executeUpdate("DELETE FROM ArtefactTypes WHERE artefacttypename='A25 Drawing'");
    } catch (...) {}

    int before = queryCount("SELECT COUNT(*) FROM ArtefactTypes");
    ASSERT_NO_THROW(runCommand(
        "add \"artefactname:A25 Drawing,artefactdescription:Test drawings\""));
    int after = queryCount("SELECT COUNT(*) FROM ArtefactTypes");
    EXPECT_EQ(after, before + 1);
}

// A-32 — Add Milestone
TEST_F(AddTest, A32_AddMilestone) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Milestones");

    try {
        db->executeUpdate("DELETE FROM Milestones WHERE milestonename='A32 Drawing Approval'");
    } catch (...) {}

    int before = queryCount("SELECT COUNT(*) FROM Milestones");
    ASSERT_NO_THROW(runCommand("add \"milestonename:A32 Drawing Approval\""));
    int after = queryCount("SELECT COUNT(*) FROM Milestones");
    EXPECT_EQ(after, before + 1);
}

// A-33 — Add Milestone Step with valid ratio
TEST_F(AddTest, A33_AddMilestoneStepValidRatio) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Milestones");

    // Ensure a milestone exists
    try {
        runCommand("add \"milestonename:A33 Milestone\"");
    } catch (const DuplicateEntryError&) {}

    commands::select(*db, pth, "A33 Milestone");

    int before = queryCount("SELECT COUNT(*) FROM MilestoneSteps");
    ASSERT_NO_THROW(
        runCommand("add \"milestonestepname:A33 Step,progressratio:0.50\""));
    int after = queryCount("SELECT COUNT(*) FROM MilestoneSteps");
    EXPECT_EQ(after, before + 1);
}

// A-34 — Add Milestone Step with ratio > 1 raises ValidationError
TEST_F(AddTest, A34_AddMilestoneStepRatioOutOfRange) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Milestones");

    try {
        runCommand("add \"milestonename:A34 Milestone\"");
    } catch (const DuplicateEntryError&) {}

    commands::select(*db, pth, "A34 Milestone");

    EXPECT_THROW(
        runCommand("add \"milestonestepname:BadStep,progressratio:1.50\""),
        ValidationError);
}

// A-35 — Add Project Team Member
TEST_F(AddTest, A35_AddProjectTeamMember) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Team");

    // Use Christopher Lee who is seeded
    int before = queryCount("SELECT COUNT(*) FROM ProjectTeamMembers");
    try {
        ASSERT_NO_THROW(runCommand("add \"employeename:Christopher Lee,role:1\""));
        int after = queryCount("SELECT COUNT(*) FROM ProjectTeamMembers");
        EXPECT_GE(after, before);
    } catch (const DuplicateEntryError&) {
        // Already a team member — that's acceptable
    }
}

// A-38 — Positional CSV add (department)
TEST_F(AddTest, A38_PositionalAddDepartment) {
    navigateTo({"Organization", "Departments"});

    try {
        db->executeUpdate("DELETE FROM Departments WHERE departmentname='A38 Mechanical'");
    } catch (...) {}

    int before = queryCount("SELECT COUNT(*) FROM Departments");
    ASSERT_NO_THROW(runCommand("add \"A38 Mechanical\""));
    int after = queryCount("SELECT COUNT(*) FROM Departments");
    EXPECT_EQ(after, before + 1);
}

// A-39 — Positional CSV add (employee)
TEST_F(AddTest, A39_PositionalAddEmployee) {
    navigateTo({"Organization", "Departments"});
    commands::select(*db, pth, "1");

    const std::string email = "a39.tom@osoem.test";
    try {
        db->executeUpdate("DELETE FROM Employees WHERE email='" + email + "'");
    } catch (...) {}

    int before = queryCount("SELECT COUNT(*) FROM Employees");
    ASSERT_NO_THROW(runCommand("add \"Tom,A39Jones," + email + ",pass123\""));
    int after = queryCount("SELECT COUNT(*) FROM Employees");
    EXPECT_EQ(after, before + 1);

    // Password must be hashed
    std::string hash = queryString(
        "SELECT password FROM Employees WHERE email='" + email + "'");
    EXPECT_EQ(hash.size(), 64u);
}
