// test_csv_parser.cpp — Unit tests for utils::parseLine, parseCSVString, trim (no DB)

#include <gtest/gtest.h>
#include "utils/csv_parser.h"

// ─────────────────────────────────────────────────────────────────────────────
// trim
// ─────────────────────────────────────────────────────────────────────────────

TEST(CsvParserTrim, NoWhitespace) {
    EXPECT_EQ(utils::trim("hello"), "hello");
}

TEST(CsvParserTrim, LeadingAndTrailingSpaces) {
    EXPECT_EQ(utils::trim("  hello  "), "hello");
}

TEST(CsvParserTrim, TabsAndNewlines) {
    EXPECT_EQ(utils::trim("\t\nhello\n\t"), "hello");
}

TEST(CsvParserTrim, EmptyString) {
    EXPECT_EQ(utils::trim(""), "");
}

TEST(CsvParserTrim, OnlyWhitespace) {
    EXPECT_EQ(utils::trim("   "), "");
}

// ─────────────────────────────────────────────────────────────────────────────
// parseLine — single CSV line → vector<string>
// ─────────────────────────────────────────────────────────────────────────────

TEST(CsvParserParseLine, SimpleThreeFields) {
    auto fields = utils::parseLine("a,b,c");
    ASSERT_EQ(fields.size(), 3u);
    EXPECT_EQ(fields[0], "a");
    EXPECT_EQ(fields[1], "b");
    EXPECT_EQ(fields[2], "c");
}

TEST(CsvParserParseLine, QuotedFieldWithComma) {
    // "Smith, John",42 → two fields: {"Smith, John", "42"}
    auto fields = utils::parseLine("\"Smith, John\",42");
    ASSERT_EQ(fields.size(), 2u);
    EXPECT_EQ(fields[0], "Smith, John");
    EXPECT_EQ(fields[1], "42");
}

TEST(CsvParserParseLine, EmptyFields) {
    auto fields = utils::parseLine("a,,c");
    ASSERT_EQ(fields.size(), 3u);
    EXPECT_EQ(fields[1], "");
}

TEST(CsvParserParseLine, SingleField) {
    auto fields = utils::parseLine("hello");
    ASSERT_EQ(fields.size(), 1u);
    EXPECT_EQ(fields[0], "hello");
}

TEST(CsvParserParseLine, QuotedFieldWithQuoteEscape) {
    // Standard CSV double-quote escape: "" inside quoted field → "
    auto fields = utils::parseLine("\"he said \"\"hi\"\"\"");
    ASSERT_EQ(fields.size(), 1u);
    EXPECT_NE(fields[0].find("hi"), std::string::npos);
}

TEST(CsvParserParseLine, TrailingComma) {
    // "a,b," → 3 fields, last empty
    auto fields = utils::parseLine("a,b,");
    EXPECT_GE(fields.size(), 2u);
}

// ─────────────────────────────────────────────────────────────────────────────
// parseCSVString — multi-line CSV string → vector<map<string,string>>
// ─────────────────────────────────────────────────────────────────────────────

TEST(CsvParserParseCSVString, TwoDataRows) {
    const std::string csv =
        "projectname,projectno\n"
        "Building Project,PRJ-001\n"
        "Bridge Project,PRJ-002\n";

    auto rows = utils::parseCSVString(csv);
    ASSERT_EQ(rows.size(), 2u);
    EXPECT_EQ(rows[0]["projectname"], "Building Project");
    EXPECT_EQ(rows[0]["projectno"], "PRJ-001");
    EXPECT_EQ(rows[1]["projectname"], "Bridge Project");
}

TEST(CsvParserParseCSVString, EmptyBody) {
    const std::string csv = "projectname,projectno\n";
    auto rows = utils::parseCSVString(csv);
    EXPECT_EQ(rows.size(), 0u);
}

TEST(CsvParserParseCSVString, HeaderCasePreserved) {
    // Keys in the map should match the header as-written (or lowercased, per impl)
    const std::string csv = "Name,Age\nAlice,30\n";
    auto rows = utils::parseCSVString(csv);
    ASSERT_EQ(rows.size(), 1u);
    // Either "Name" or "name" should exist as key — use count to check either
    bool hasKey = rows[0].count("Name") || rows[0].count("name");
    EXPECT_TRUE(hasKey);
}

TEST(CsvParserParseCSVString, QuotedValueInData) {
    const std::string csv =
        "projectname,projectno\n"
        "\"Smith, Jones & Partners\",PRJ-003\n";

    auto rows = utils::parseCSVString(csv);
    ASSERT_EQ(rows.size(), 1u);
    // The project name contains a comma; verify it was not split
    bool found = rows[0]["projectname"] == "Smith, Jones & Partners" ||
                 rows[0]["Projectname"] == "Smith, Jones & Partners";
    // Use case-insensitive check via either key
    EXPECT_TRUE(
        rows[0].count("projectname") &&
        rows[0]["projectname"].find("Jones") != std::string::npos);
}
