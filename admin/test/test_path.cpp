// test_path.cpp — Unit tests for data::Path class (no DB required)

#include <gtest/gtest.h>
#include "data/path.h"

using data::Path;

// ─────────────────────────────────────────────────────────────────────────────
// Initial state
// ─────────────────────────────────────────────────────────────────────────────

TEST(PathTest, DefaultConstructorIsRoot) {
    Path pth;
    EXPECT_EQ(pth.current_type(), Path::ROOT);
    EXPECT_EQ(pth.to_string(), "~/");
}

// ─────────────────────────────────────────────────────────────────────────────
// move_forward — simple push with explicit type
// ─────────────────────────────────────────────────────────────────────────────

TEST(PathTest, MoveForwardSingleStep) {
    Path pth;
    pth.move_forward("Projects", 0, Path::PROJECTS_FOLDER);
    EXPECT_EQ(pth.current_type(), Path::PROJECTS_FOLDER);
    EXPECT_NE(pth.to_string().find("Projects"), std::string::npos);
}

TEST(PathTest, MoveForwardMultipleSteps) {
    Path pth;
    pth.move_forward("Projects", 0, Path::PROJECTS_FOLDER);
    pth.move_forward("Building Project", 1, Path::PROJECT);
    EXPECT_EQ(pth.current_type(), Path::PROJECT);
    EXPECT_EQ(pth.current_id(), 1);
    EXPECT_EQ(pth.current_name(), "Building Project");
}

// ─────────────────────────────────────────────────────────────────────────────
// move_back — pop from stack
// ─────────────────────────────────────────────────────────────────────────────

TEST(PathTest, MoveBackOneLevelRestoresParent) {
    Path pth;
    pth.move_forward("Projects", 0, Path::PROJECTS_FOLDER);
    pth.move_forward("Building Project", 1, Path::PROJECT);
    pth.move_back();
    EXPECT_EQ(pth.current_type(), Path::PROJECTS_FOLDER);
}

TEST(PathTest, MoveBackAtRootIsNoop) {
    Path pth;
    // Should not throw or crash when already at root
    EXPECT_NO_THROW(pth.move_back());
    EXPECT_EQ(pth.current_type(), Path::ROOT);
}

// ─────────────────────────────────────────────────────────────────────────────
// reset — return to root
// ─────────────────────────────────────────────────────────────────────────────

TEST(PathTest, ResetFromDeepHierarchy) {
    Path pth;
    pth.move_forward("Projects", 0, Path::PROJECTS_FOLDER);
    pth.move_forward("Building Project", 1, Path::PROJECT);
    pth.move_forward("Activity Categories", 0, Path::ACTIVITIES_FOLDER);
    pth.reset();
    EXPECT_EQ(pth.current_type(), Path::ROOT);
    EXPECT_EQ(pth.to_string(), "~/");
}

// ─────────────────────────────────────────────────────────────────────────────
// to_string — path string formatting
// ─────────────────────────────────────────────────────────────────────────────

TEST(PathTest, ToStringAtRoot) {
    Path pth;
    EXPECT_EQ(pth.to_string(), "~/");
}

TEST(PathTest, ToStringAfterNavigation) {
    Path pth;
    pth.move_forward("Projects", 0, Path::PROJECTS_FOLDER);
    pth.move_forward("Building Project", 1, Path::PROJECT);
    std::string s = pth.to_string();
    EXPECT_NE(s.find("Projects"), std::string::npos);
    EXPECT_NE(s.find("Building Project"), std::string::npos);
}

// ─────────────────────────────────────────────────────────────────────────────
// getIdByType — search path stack for a particular item type
// ─────────────────────────────────────────────────────────────────────────────

TEST(PathTest, GetIdByTypeFound) {
    Path pth;
    pth.move_forward("Projects", 0, Path::PROJECTS_FOLDER);
    pth.move_forward("Building Project", 7, Path::PROJECT);
    pth.move_forward("Activity Categories", 0, Path::ACTIVITIES_FOLDER);
    EXPECT_EQ(pth.getIdByType(Path::PROJECT), 7);
}

TEST(PathTest, GetIdByTypeNotFound) {
    Path pth;
    pth.move_forward("Projects", 0, Path::PROJECTS_FOLDER);
    EXPECT_EQ(pth.getIdByType(Path::PROJECT), -1);
}

TEST(PathTest, GetIdByTypeAtRoot) {
    Path pth;
    EXPECT_EQ(pth.getIdByType(Path::PROJECT), -1);
}

// ─────────────────────────────────────────────────────────────────────────────
// current_id / current_name / current_type at root
// ─────────────────────────────────────────────────────────────────────────────

TEST(PathTest, CurrentAtRoot) {
    Path pth;
    EXPECT_EQ(pth.current_id(), 0);
    EXPECT_EQ(pth.current_type(), Path::ROOT);
}

// ─────────────────────────────────────────────────────────────────────────────
// move_forward without explicit type (inferred)
// ─────────────────────────────────────────────────────────────────────────────

TEST(PathTest, MoveForwardWithoutExplicitType) {
    Path pth;
    // The overload without ItemType should not crash
    EXPECT_NO_THROW(pth.move_forward("SomeItem", 5));
}

// ─────────────────────────────────────────────────────────────────────────────
// Deep hierarchy — verify IDs survive multiple move_forward/move_back cycles
// ─────────────────────────────────────────────────────────────────────────────

TEST(PathTest, DeepHierarchyIdRetrieval) {
    Path pth;
    pth.move_forward("Projects", 0, Path::PROJECTS_FOLDER);
    pth.move_forward("Building Project", 1, Path::PROJECT);
    pth.move_forward("Activity Categories", 0, Path::ACTIVITIES_FOLDER);
    pth.move_forward("Structural", 3, Path::CATEGORY);
    pth.move_forward("Design Development", 5, Path::SUBCATEGORY);

    EXPECT_EQ(pth.getIdByType(Path::PROJECT), 1);
    EXPECT_EQ(pth.getIdByType(Path::CATEGORY), 3);
    EXPECT_EQ(pth.getIdByType(Path::SUBCATEGORY), 5);

    pth.move_back(); // back to CATEGORY
    EXPECT_EQ(pth.current_type(), Path::CATEGORY);
    EXPECT_EQ(pth.getIdByType(Path::SUBCATEGORY), -1);
}
