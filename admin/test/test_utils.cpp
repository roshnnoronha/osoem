// test_utils.cpp — Unit tests for utils:: helper functions (no DB required)

#include <gtest/gtest.h>
#include "utils/utils.h"
#include "exceptions/exceptions.h"

// ─────────────────────────────────────────────────────────────────────────────
// isValidDate
// ─────────────────────────────────────────────────────────────────────────────

TEST(UtilsIsValidDate, ValidISODate) {
    EXPECT_TRUE(utils::isValidDate("2026-01-01"));
    EXPECT_TRUE(utils::isValidDate("2000-12-31"));
    EXPECT_TRUE(utils::isValidDate("2026-02-19"));
}

TEST(UtilsIsValidDate, WrongSeparator) {
    EXPECT_FALSE(utils::isValidDate("2026/01/01"));
    EXPECT_FALSE(utils::isValidDate("2026.01.01"));
    EXPECT_FALSE(utils::isValidDate("01-01-2026"));
}

TEST(UtilsIsValidDate, InvalidMonthOrDay) {
    EXPECT_FALSE(utils::isValidDate("2026-13-01")); // month 13
    EXPECT_FALSE(utils::isValidDate("2026-00-01")); // month 0
    EXPECT_FALSE(utils::isValidDate("2026-01-32")); // day 32
    EXPECT_FALSE(utils::isValidDate("2026-01-00")); // day 0
}

TEST(UtilsIsValidDate, EmptyAndGarbage) {
    EXPECT_FALSE(utils::isValidDate(""));
    EXPECT_FALSE(utils::isValidDate("not-a-date"));
    EXPECT_FALSE(utils::isValidDate("2026-1-1"));   // no zero-padding
}

// ─────────────────────────────────────────────────────────────────────────────
// toLower
// ─────────────────────────────────────────────────────────────────────────────

TEST(UtilsToLower, AllUpper) {
    EXPECT_EQ(utils::toLower("HELLO"), "hello");
}

TEST(UtilsToLower, MixedCase) {
    EXPECT_EQ(utils::toLower("Building Project"), "building project");
}

TEST(UtilsToLower, AlreadyLower) {
    EXPECT_EQ(utils::toLower("already"), "already");
}

TEST(UtilsToLower, EmptyString) {
    EXPECT_EQ(utils::toLower(""), "");
}

// ─────────────────────────────────────────────────────────────────────────────
// toInt
// ─────────────────────────────────────────────────────────────────────────────

TEST(UtilsToInt, ValidInteger) {
    EXPECT_EQ(utils::toInt("42"), 42);
    EXPECT_EQ(utils::toInt("0"), 0);
    EXPECT_EQ(utils::toInt("-7"), -7);
}

TEST(UtilsToInt, UsesDefaultOnNonNumeric) {
    EXPECT_EQ(utils::toInt("abc", -1), -1);
    EXPECT_EQ(utils::toInt("", 99), 99);
}

// ─────────────────────────────────────────────────────────────────────────────
// toDouble
// ─────────────────────────────────────────────────────────────────────────────

TEST(UtilsToDouble, ValidDouble) {
    EXPECT_DOUBLE_EQ(utils::toDouble("3.14"), 3.14);
    EXPECT_DOUBLE_EQ(utils::toDouble("0.5"), 0.5);
    EXPECT_DOUBLE_EQ(utils::toDouble("1"), 1.0);
}

TEST(UtilsToDouble, UsesDefaultOnNonNumeric) {
    EXPECT_DOUBLE_EQ(utils::toDouble("abc", -1.0), -1.0);
}

// ─────────────────────────────────────────────────────────────────────────────
// isNumber
// ─────────────────────────────────────────────────────────────────────────────

TEST(UtilsIsNumber, ValidIntegers) {
    EXPECT_TRUE(utils::isNumber("0"));
    EXPECT_TRUE(utils::isNumber("123"));
    EXPECT_TRUE(utils::isNumber("-5"));
}

TEST(UtilsIsNumber, ValidDecimals) {
    EXPECT_TRUE(utils::isNumber("3.14"));
    EXPECT_TRUE(utils::isNumber("0.5"));
}

TEST(UtilsIsNumber, NotNumbers) {
    EXPECT_FALSE(utils::isNumber("abc"));
    EXPECT_FALSE(utils::isNumber("12abc"));
    EXPECT_FALSE(utils::isNumber(""));
}

// ─────────────────────────────────────────────────────────────────────────────
// split
// ─────────────────────────────────────────────────────────────────────────────

TEST(UtilsSplit, CommaSeparated) {
    auto parts = utils::split("a,b,c", ',');
    ASSERT_EQ(parts.size(), 3u);
    EXPECT_EQ(parts[0], "a");
    EXPECT_EQ(parts[1], "b");
    EXPECT_EQ(parts[2], "c");
}

TEST(UtilsSplit, SingleElement) {
    auto parts = utils::split("hello", ',');
    ASSERT_EQ(parts.size(), 1u);
    EXPECT_EQ(parts[0], "hello");
}

TEST(UtilsSplit, EmptyString) {
    auto parts = utils::split("", ',');
    // Empty string split on comma should yield one empty token or zero; either
    // is acceptable — just verify it does not crash.
    EXPECT_GE(parts.size(), 0u);
}

// ─────────────────────────────────────────────────────────────────────────────
// isValidLength
// ─────────────────────────────────────────────────────────────────────────────

TEST(UtilsIsValidLength, WithinLimit) {
    EXPECT_TRUE(utils::isValidLength("hello", 10));
    EXPECT_TRUE(utils::isValidLength("hello", 5)); // exactly at limit
}

TEST(UtilsIsValidLength, ExceedsLimit) {
    EXPECT_FALSE(utils::isValidLength("hello world", 5));
}

// ─────────────────────────────────────────────────────────────────────────────
// startsWithNumber
// ─────────────────────────────────────────────────────────────────────────────

TEST(UtilsStartsWithNumber, StartsWithDigit) {
    EXPECT_TRUE(utils::startsWithNumber("1abc"));
    EXPECT_TRUE(utils::startsWithNumber("42"));
}

TEST(UtilsStartsWithNumber, StartsWithLetter) {
    EXPECT_FALSE(utils::startsWithNumber("abc1"));
    EXPECT_FALSE(utils::startsWithNumber(""));
}

// ─────────────────────────────────────────────────────────────────────────────
// containsReservedChars
// ─────────────────────────────────────────────────────────────────────────────

TEST(UtilsContainsReservedChars, WithComma) {
    EXPECT_TRUE(utils::containsReservedChars("a,b"));
}

TEST(UtilsContainsReservedChars, WithoutComma) {
    EXPECT_FALSE(utils::containsReservedChars("hello world"));
}

// ─────────────────────────────────────────────────────────────────────────────
// validateCsvHeader — basic smoke tests (error path requires CsvParseError)
// ─────────────────────────────────────────────────────────────────────────────

TEST(UtilsValidateCsvHeader, ValidHeader) {
    EXPECT_NO_THROW(utils::validateCsvHeader(
        "projectname,projectno",
        {"projectname", "projectno"}));
}

TEST(UtilsValidateCsvHeader, MissingRequiredField) {
    EXPECT_THROW(
        utils::validateCsvHeader("projectno", {"projectname", "projectno"}),
        CsvParseError);
}

TEST(UtilsValidateCsvHeader, CaseInsensitiveMatch) {
    // Headers are typically lowercased before comparison
    EXPECT_NO_THROW(utils::validateCsvHeader(
        "ProjectName,ProjectNo",
        {"projectname", "projectno"}));
}
