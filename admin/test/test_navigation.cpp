// test_navigation.cpp — Integration tests for select/navigate commands
// Covers TEST_PLAN sections N-01 to N-10

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "commands/select.h"
#include "exceptions/exceptions.h"

class NavigationTest : public OsoemTestFixture {};

// N-01 — Navigate to root from anywhere
TEST_F(NavigationTest, N01_NavigateToRoot) {
    navigateTo({"Projects"});
    ASSERT_NE(pth.current_type(), data::Path::ROOT);

    commands::select(*db, pth, "~");
    EXPECT_EQ(pth.current_type(), data::Path::ROOT);
    EXPECT_EQ(pth.to_string(), "~/");
}

// N-02 — Navigate up one level with ".."
TEST_F(NavigationTest, N02_NavigateUpOneLevel) {
    navigateTo({"Projects"});
    data::Path::ItemType parentType = data::Path::ROOT;

    // Go into Projects folder, then back up
    EXPECT_EQ(pth.current_type(), data::Path::PROJECTS_FOLDER);
    commands::select(*db, pth, "..");
    EXPECT_EQ(pth.current_type(), parentType);
}

// N-03 — Case-insensitive folder name selection
TEST_F(NavigationTest, N03_CaseInsensitiveFolderSelect) {
    // "projects" (lowercase) should enter the Projects folder
    commands::select(*db, pth, "projects");
    EXPECT_EQ(pth.current_type(), data::Path::PROJECTS_FOLDER);
}

TEST_F(NavigationTest, N03_UpperCaseFolderSelect) {
    commands::select(*db, pth, "PROJECTS");
    EXPECT_EQ(pth.current_type(), data::Path::PROJECTS_FOLDER);
}

// N-04 — Select project by quoted name
TEST_F(NavigationTest, N04_SelectProjectByName) {
    navigateTo({"Projects"});
    // "Building Project" is seeded by project.src
    ASSERT_NO_THROW(commands::select(*db, pth, "Building Project"));
    EXPECT_EQ(pth.current_type(), data::Path::PROJECT);
    EXPECT_NE(pth.to_string().find("Building Project"), std::string::npos);
}

// N-05 — Select project by numeric ID
TEST_F(NavigationTest, N05_SelectProjectById) {
    navigateTo({"Projects"});
    int projId = queryCount("SELECT projectid FROM Projects LIMIT 1");
    if (projId == 0) {
        GTEST_SKIP() << "No projects in test DB";
    }
    ASSERT_NO_THROW(commands::select(*db, pth, std::to_string(projId)));
    EXPECT_EQ(pth.current_type(), data::Path::PROJECT);
}

// N-06 — Select non-existent item raises error
TEST_F(NavigationTest, N06_SelectNonExistentItem) {
    navigateTo({"Projects"});
    EXPECT_THROW(commands::select(*db, pth, "DoesNotExist"),
                 DataError);
    // Prompt must remain at Projects folder
    EXPECT_EQ(pth.current_type(), data::Path::PROJECTS_FOLDER);
}

// N-07 — Navigate into every folder type
TEST_F(NavigationTest, N07_OrganizationFolder) {
    commands::select(*db, pth, "Organization");
    EXPECT_EQ(pth.current_type(), data::Path::ORGANIZATION_FOLDER);
}

TEST_F(NavigationTest, N07_DepartmentsFolder) {
    navigateTo({"Organization"});
    commands::select(*db, pth, "Departments");
    EXPECT_EQ(pth.current_type(), data::Path::DEPARTMENTS_FOLDER);
}

TEST_F(NavigationTest, N07_ProjectsFolder) {
    commands::select(*db, pth, "Projects");
    EXPECT_EQ(pth.current_type(), data::Path::PROJECTS_FOLDER);
}

TEST_F(NavigationTest, N07_ActivityCategoriesFolder) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Activity Categories");
    EXPECT_EQ(pth.current_type(), data::Path::ACTIVITIES_FOLDER);
}

TEST_F(NavigationTest, N07_ArtefactTypesFolder) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Artefact Types");
    EXPECT_EQ(pth.current_type(), data::Path::ARTEFACT_TYPES_FOLDER);
}

TEST_F(NavigationTest, N07_MilestonesFolder) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Milestones");
    EXPECT_EQ(pth.current_type(), data::Path::MILESTONES_FOLDER);
}

TEST_F(NavigationTest, N07_TeamFolder) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    commands::select(*db, pth, "Team");
    EXPECT_EQ(pth.current_type(), data::Path::TEAM_FOLDER);
}

// N-10 — Select "." (current directory) is a no-op
TEST_F(NavigationTest, N10_SelectDotIsNoOp) {
    navigateTo({"Projects"});
    data::Path::ItemType typeBefore = pth.current_type();
    std::string pathBefore = pth.to_string();

    commands::select(*db, pth, ".");
    EXPECT_EQ(pth.current_type(), typeBefore);
    EXPECT_EQ(pth.to_string(), pathBefore);
}

// Additional — Navigate up from root is a no-op (no crash)
TEST_F(NavigationTest, NavigateUpFromRootIsNoOp) {
    EXPECT_NO_THROW(commands::select(*db, pth, ".."));
    EXPECT_EQ(pth.current_type(), data::Path::ROOT);
}

// N-04 — Folder name is case-insensitive (Team vs team)
TEST_F(NavigationTest, N04_CaseInsensitiveTeamFolder) {
    navigateTo({"Projects"});
    commands::select(*db, pth, "Building Project");
    ASSERT_NO_THROW(commands::select(*db, pth, "team"));
    EXPECT_EQ(pth.current_type(), data::Path::TEAM_FOLDER);
}

// N-11 — Multi-level path from root (~/Projects/Building Project)
TEST_F(NavigationTest, N11_MultiLevelPathFromRoot) {
    // Use slash-separated path starting with ~ to navigate multiple levels at once
    ASSERT_NO_THROW(commands::select(*db, pth, "~/Projects/Building Project"));
    EXPECT_EQ(pth.current_type(), data::Path::PROJECT);
    EXPECT_NE(pth.to_string().find("Building Project"), std::string::npos);
}

// N-12 — Multi-level relative path without ~ prefix
TEST_F(NavigationTest, N12_MultiLevelRelativePath) {
    // From ROOT, navigate Organization/Departments in one command
    ASSERT_NO_THROW(commands::select(*db, pth, "Organization/Departments"));
    EXPECT_EQ(pth.current_type(), data::Path::DEPARTMENTS_FOLDER);
}

// N-13 — Double-dot segments work inside slash-separated path
TEST_F(NavigationTest, N13_DoubleDotInPath) {
    // Navigate to Projects, then ../Projects should end at PROJECTS_FOLDER
    navigateTo({"Projects"});
    ASSERT_NO_THROW(commands::select(*db, pth, "../Projects"));
    EXPECT_EQ(pth.current_type(), data::Path::PROJECTS_FOLDER);
}
