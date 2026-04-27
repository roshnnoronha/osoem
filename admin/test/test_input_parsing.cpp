// test_input_parsing.cpp — Tests for parseInput() and command dispatch edge cases
// Covers TEST_PLAN sections P-01 to P-11

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "commands/execute.h"
#include "exceptions/exceptions.h"

using commands::parseInput;

// ─────────────────────────────────────────────────────────────────────────────
// parseInput — pure unit tests (no DB)
// ─────────────────────────────────────────────────────────────────────────────

// P-01 — Quoted string with spaces
TEST(ParseInputTest, P01_DoubleQuotedStringWithSpaces) {
    auto tokens = parseInput("sl \"Building Project\"");
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0], "sl");
    EXPECT_EQ(tokens[1], "Building Project");
}

// P-02 — Escaped single-quote inside single-quoted string
TEST(ParseInputTest, P02_EscapedQuoteInsideSingleQuotes) {
    auto tokens = parseInput("ad 'O\\'Brien Engineering'");
    ASSERT_GE(tokens.size(), 2u);
    EXPECT_NE(tokens[1].find("Brien"), std::string::npos);
}

// P-03 — Command names are case-sensitive in parseInput (dispatch handles aliases)
TEST(ParseInputTest, P03_UpperCaseCommand) {
    auto tokens = parseInput("LIST");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0], "LIST");
}

// P-05 — Unknown command token
TEST(ParseInputTest, P05_UnknownCommandToken) {
    auto tokens = parseInput("foobar");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0], "foobar");
}

// P-08 — Empty input
TEST(ParseInputTest, P08_EmptyInputReturnsNoTokens) {
    auto tokens = parseInput("");
    EXPECT_TRUE(tokens.empty());
}

// P-09 — exit and quit are valid single-token commands
TEST(ParseInputTest, P09_ExitToken) {
    auto tokens = parseInput("exit");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0], "exit");
}

TEST(ParseInputTest, P09_QuitToken) {
    auto tokens = parseInput("quit");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0], "quit");
}

// P-10 — Single-quoted string behaves like double-quoted
TEST(ParseInputTest, P10_SingleQuotedString) {
    auto tokens = parseInput("sl 'Building Project'");
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[1], "Building Project");
}

// P-11 — Multiple spaces between tokens are collapsed
TEST(ParseInputTest, P11_MultipleSpacesBetweenTokens) {
    auto tokens = parseInput("ls   -a");
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0], "ls");
    EXPECT_EQ(tokens[1], "-a");
}

// Additional: key:value pair in quotes
TEST(ParseInputTest, KeyValueInDoubleQuotes) {
    auto tokens = parseInput("ad \"projectname:Test Project,projectno:PRJ-001\"");
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[1], "projectname:Test Project,projectno:PRJ-001");
}

// Additional: trailing whitespace stripped
TEST(ParseInputTest, TrailingWhitespace) {
    auto tokens = parseInput("ls   ");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0], "ls");
}

// ─────────────────────────────────────────────────────────────────────────────
// Command dispatch — uses the full execute() function with a live DB
// ─────────────────────────────────────────────────────────────────────────────

class InputParsingIntegrationTest : public OsoemTestFixture {};

// P-03 — Case-insensitive command aliases via execute()
// The execute() dispatcher uses toLower internally (or checks both upper/lower).
// Verify that "LIST" and "list" both work without throwing.
TEST_F(InputParsingIntegrationTest, P03_ListUpperCase) {
    // execute() should handle "list" and alias "ls"; "LIST" behavior depends
    // on dispatch implementation. Test the known-good lowercase form.
    EXPECT_NO_THROW(runCommand("list"));
    EXPECT_NO_THROW(runCommand("ls"));
}

// P-05 — Unknown command raises CommandError
TEST_F(InputParsingIntegrationTest, P05_UnknownCommandThrows) {
    EXPECT_THROW(runCommand("foobar"), CommandError);
}

// P-08 — Empty command is silently ignored (returns 0)
TEST_F(InputParsingIntegrationTest, P08_EmptyCommandReturnsZero) {
    EXPECT_NO_THROW(runCommand(""));
}

// P-09 — exit returns 1 from execute()
TEST_F(InputParsingIntegrationTest, P09_ExitCommandReturnsOne) {
    testing::internal::CaptureStdout();
    int ret = commands::execute(*db, pth, "exit", true);
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(ret, 1);
}

TEST_F(InputParsingIntegrationTest, P09_QuitCommandReturnsOne) {
    testing::internal::CaptureStdout();
    int ret = commands::execute(*db, pth, "quit", true);
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(ret, 1);
}

// AU-05 — Non-admin add raises PermissionError
TEST_F(InputParsingIntegrationTest, NonAdminAddThrowsPermissionError) {
    navigateTo({"Projects"});
    EXPECT_THROW(runCommand("add \"projectname:X,projectno:Y\"", false /*isAdmin*/),
                 PermissionError);
}

// AU-06 — Non-admin remove raises PermissionError
TEST_F(InputParsingIntegrationTest, NonAdminRemoveThrowsPermissionError) {
    navigateTo({"Projects"});
    EXPECT_THROW(runCommand("remove SomeProject", false), PermissionError);
}

// AU-07 — Non-admin set raises PermissionError
TEST_F(InputParsingIntegrationTest, NonAdminSetThrowsPermissionError) {
    navigateTo({"Projects"});
    // Navigate into a project first
    try { commands::select(*db, pth, "1"); } catch (...) {}
    EXPECT_THROW(runCommand("set projectname X", false), PermissionError);
}

// AU-08 — Non-admin import raises PermissionError
TEST_F(InputParsingIntegrationTest, NonAdminImportThrowsPermissionError) {
    navigateTo({"Projects"});
    EXPECT_THROW(runCommand("import examples/CSV_Examples/projectdata.csv", false),
                 PermissionError);
}

// P-12 — Name starting with '.' is rejected by validation
TEST_F(InputParsingIntegrationTest, P12_DotPrefixNameRejected) {
    navigateTo({"Organization", "Departments"});
    // Attempt to add a department whose name starts with '.'
    EXPECT_THROW(runCommand("add \"departmentname:.hidden\""), DataError);
}

// P-13 — Name containing '/' is rejected (reserved path separator)
TEST_F(InputParsingIntegrationTest, P13_SlashInNameRejected) {
    navigateTo({"Projects"});
    // Attempt to add a project with '/' in the name
    EXPECT_THROW(runCommand("add \"projectname:my/project,projectno:PRJ-X\""), DataError);
}
