// test_batch.cpp — Integration tests for batch (script file) execution
// Covers TEST_PLAN sections B-01 to B-07

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "commands/execute.h"
#include "exceptions/exceptions.h"

#include <fstream>
#include <cstdio>
#include <string>

class BatchTest : public OsoemTestFixture {};

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

static std::string writeSrcFile(const std::string& basename,
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

// B-01 — Run valid .src file (departments + project)
TEST_F(BatchTest, B01_RunValidSrcFile) {
    // After SetUpTestSuite, departments.src and project.src were already run.
    // Here we confirm the result: Building Project exists.
    int count = queryCount(
        "SELECT COUNT(*) FROM Projects WHERE projectname='Building Project'");
    EXPECT_GE(count, 1);
}

// B-02 — Comment lines (#) are ignored (no CommandError)
TEST_F(BatchTest, B02_CommentLinesAreIgnored) {
    std::string path = writeSrcFile("b02_comments.src",
        "# This is a comment\n"
        "# Another comment\n"
        "cd ~\n"
        "# End\n");

    EXPECT_NO_THROW(commands::batchExecute(*db, pth, path));
    std::remove(path.c_str());
}

// B-03 — Empty lines are skipped (no crash)
TEST_F(BatchTest, B03_EmptyLinesAreSkipped) {
    std::string path = writeSrcFile("b03_blanks.src",
        "\n"
        "cd ~\n"
        "\n"
        "ls\n"
        "\n");

    EXPECT_NO_THROW(commands::batchExecute(*db, pth, path));
    std::remove(path.c_str());
}

// B-04 — Script stops at first error; subsequent commands not executed
TEST_F(BatchTest, B04_ScriptStopsAtFirstError) {
    // Create a script where the 2nd command fails, and check that the 3rd
    // command ("cd Organization") never runs by verifying path state.
    std::string path = writeSrcFile("b04_error.src",
        "cd ~\n"
        "cd Projects\n"
        "rm \"Nonexistent Project ZZZZ\"\n"  // This will fail → stop
        "cd Organization\n");                 // Must NOT be reached

    // batchExecute catches DataError and prints it; does NOT propagate.
    // After the error the function returns early.
    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    EXPECT_NO_THROW(commands::batchExecute(*db, pth, path));
    testing::internal::GetCapturedStdout();
    std::string err = testing::internal::GetCapturedStderr();

    // Path should NOT be at Organization (execution stopped before that line)
    EXPECT_NE(pth.current_type(), data::Path::ORGANIZATION_FOLDER);
    // Error message should have been printed to stderr
    EXPECT_NE(err.find("Error"), std::string::npos);

    std::remove(path.c_str());
}

// B-05 — Batch mode always runs as admin (no PermissionError from write commands)
TEST_F(BatchTest, B05_BatchModeIsAdmin) {
    // batchExecute internally calls execute(..., isAdmin=1)
    std::string path = writeSrcFile("b05_admin.src",
        "cd ~\n"
        "cd Projects\n"
        "ad \"projectname:B05 Batch Project,projectno:B05-001\"\n"
        "rm \"B05 Batch Project\"\n");

    // Should not throw PermissionError
    EXPECT_NO_THROW(commands::batchExecute(*db, pth, path));
    std::remove(path.c_str());
}

// B-06 — Navigation state persists across script lines
TEST_F(BatchTest, B06_NavigationStatePersistsAcrossLines) {
    // Navigate deep and verify the add command lands at the right location
    std::string path = writeSrcFile("b06_nav_state.src",
        "cd ~\n"
        "cd Projects\n"
        "cd \"Building Project\"\n"
        "cd Milestones\n"
        "ad \"milestonename:B06 Batch Milestone\"\n");

    EXPECT_NO_THROW(commands::batchExecute(*db, pth, path));

    int count = queryCount(
        "SELECT COUNT(*) FROM Milestones WHERE milestonename='B06 Batch Milestone'");
    EXPECT_GE(count, 1);

    std::remove(path.c_str());
}

// B-07 — Non-existent script file raises FileIOError
TEST_F(BatchTest, B07_NonExistentScriptFile) {
    EXPECT_THROW(commands::batchExecute(*db, pth, "no_such_file_osoem.src"),
                 FileIOError);
}
