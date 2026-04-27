// test_help.cpp — Tests for the help command
// Covers TEST_PLAN section 13 (H-01 to H-10)

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "exceptions/exceptions.h"

class HelpTest : public OsoemTestFixture {};

// H-01 — help with no args prints general command list without throwing
TEST_F(HelpTest, H01_HelpNoArgs) {
    std::string out;
    ASSERT_NO_THROW(out = runCommand("help"));
    // General help must mention key commands
    EXPECT_NE(out.find("list"), std::string::npos);
    EXPECT_NE(out.find("select"), std::string::npos);
    EXPECT_NE(out.find("add"), std::string::npos);
    EXPECT_NE(out.find("import"), std::string::npos);
    EXPECT_NE(out.find("export"), std::string::npos);
}

// H-02 — help list (and alias ls) prints list-specific documentation
TEST_F(HelpTest, H02_HelpList) {
    std::string out;
    ASSERT_NO_THROW(out = runCommand("help list"));
    EXPECT_NE(out.find("-a"), std::string::npos);

    ASSERT_NO_THROW(out = runCommand("help ls"));
    EXPECT_NE(out.find("-a"), std::string::npos);
}

// H-03 — help select (and aliases cd, sl) prints navigation documentation
TEST_F(HelpTest, H03_HelpSelect) {
    std::string out;
    ASSERT_NO_THROW(out = runCommand("help select"));
    // Must mention the ~ and .. special args
    EXPECT_NE(out.find("~"), std::string::npos);
    EXPECT_NE(out.find(".."), std::string::npos);

    ASSERT_NO_THROW(runCommand("help cd"));
    ASSERT_NO_THROW(runCommand("help sl"));
}

// H-04 — help add (and alias ad) prints add documentation
TEST_F(HelpTest, H04_HelpAdd) {
    ASSERT_NO_THROW(runCommand("help add"));
    ASSERT_NO_THROW(runCommand("help ad"));
}

// H-05 — help remove (and alias rm) prints remove documentation
TEST_F(HelpTest, H05_HelpRemove) {
    ASSERT_NO_THROW(runCommand("help remove"));
    ASSERT_NO_THROW(runCommand("help rm"));
}

// H-06 — help set (and alias st) prints set documentation
TEST_F(HelpTest, H06_HelpSet) {
    ASSERT_NO_THROW(runCommand("help set"));
    ASSERT_NO_THROW(runCommand("help st"));
}

// H-07 — help import (and alias im) prints import documentation including flags
TEST_F(HelpTest, H07_HelpImport) {
    std::string out;
    ASSERT_NO_THROW(out = runCommand("help import"));
    // Must mention the -r and -i flags
    EXPECT_NE(out.find("-r"), std::string::npos);
    EXPECT_NE(out.find("-i"), std::string::npos);

    ASSERT_NO_THROW(runCommand("help im"));
}

// H-08 — help export (and alias ex) prints export documentation
TEST_F(HelpTest, H08_HelpExport) {
    ASSERT_NO_THROW(runCommand("help export"));
    ASSERT_NO_THROW(runCommand("help ex"));
}

// H-09 — help with unknown topic does not throw; prints fallback message
TEST_F(HelpTest, H09_HelpUnknownTopic) {
    EXPECT_NO_THROW(runCommand("help unknowncommand"));
    EXPECT_NO_THROW(runCommand("help zzz"));
}

// H-10 — short alias 'h' works the same as 'help'
TEST_F(HelpTest, H10_HelpShortAlias) {
    std::string fromHelp, fromH;
    ASSERT_NO_THROW(fromHelp = runCommand("help"));
    ASSERT_NO_THROW(fromH = runCommand("h"));
    EXPECT_EQ(fromHelp, fromH);

    ASSERT_NO_THROW(runCommand("h list"));
    ASSERT_NO_THROW(runCommand("h import"));
}
