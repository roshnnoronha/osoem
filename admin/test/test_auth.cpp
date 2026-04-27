// test_auth.cpp — Unit tests for auth::generateSalt, hashPassword;
//                 integration test for auth::authenticate against osoem_test DB.

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "utils/auth.h"

// ─────────────────────────────────────────────────────────────────────────────
// generateSalt — pure unit tests (no DB)
// ─────────────────────────────────────────────────────────────────────────────

TEST(AuthGenerateSalt, LengthIs64Hex) {
    // Salt should be 32 bytes encoded as 64 hex characters
    std::string salt = auth::generateSalt();
    EXPECT_EQ(salt.size(), 64u);
}

TEST(AuthGenerateSalt, OnlyHexChars) {
    std::string salt = auth::generateSalt();
    for (char c : salt) {
        bool isHex = (c >= '0' && c <= '9') ||
                     (c >= 'a' && c <= 'f') ||
                     (c >= 'A' && c <= 'F');
        EXPECT_TRUE(isHex) << "Non-hex character: " << c;
    }
}

TEST(AuthGenerateSalt, TwoSaltsDiffer) {
    // Two independently generated salts should (with overwhelming probability) differ
    std::string s1 = auth::generateSalt();
    std::string s2 = auth::generateSalt();
    EXPECT_NE(s1, s2);
}

// ─────────────────────────────────────────────────────────────────────────────
// hashPassword — pure unit tests (no DB)
// ─────────────────────────────────────────────────────────────────────────────

TEST(AuthHashPassword, LengthIs64Hex) {
    std::string salt = auth::generateSalt();
    std::string hash = auth::hashPassword("mysecret", salt);
    EXPECT_EQ(hash.size(), 64u);
}

TEST(AuthHashPassword, DeterministicWithSameSalt) {
    std::string salt = auth::generateSalt();
    std::string h1 = auth::hashPassword("password123", salt);
    std::string h2 = auth::hashPassword("password123", salt);
    EXPECT_EQ(h1, h2);
}

TEST(AuthHashPassword, DifferentPasswordsDifferentHashes) {
    std::string salt = auth::generateSalt();
    std::string h1 = auth::hashPassword("password1", salt);
    std::string h2 = auth::hashPassword("password2", salt);
    EXPECT_NE(h1, h2);
}

TEST(AuthHashPassword, SamePasswordDifferentSaltsDifferentHashes) {
    std::string s1 = auth::generateSalt();
    std::string s2 = auth::generateSalt();
    EXPECT_NE(auth::hashPassword("password", s1),
              auth::hashPassword("password", s2));
}

TEST(AuthHashPassword, DoesNotContainPlaintext) {
    std::string salt = auth::generateSalt();
    std::string hash = auth::hashPassword("mysecretpass", salt);
    EXPECT_EQ(hash.find("mysecretpass"), std::string::npos);
}

// ─────────────────────────────────────────────────────────────────────────────
// AU-12 — Password stored as hash (integration, needs DB)
//
// Adds an employee, verifies that the password column in the DB is a 64-char
// hex hash and the passwordsalt column is a 64-char hex string.
// ─────────────────────────────────────────────────────────────────────────────

class AuthIntegrationTest : public OsoemTestFixture {};

TEST_F(AuthIntegrationTest, AU12_PasswordStoredAsHash) {
    // Navigate to the first department and add a test employee
    navigateTo({"Organization", "Departments"});
    commands::select(*db, pth, "1"); // first department

    const std::string email = "hash.test@osoem.test";
    const std::string plaintext = "mysecretpass";

    // Remove if pre-existing (idempotency)
    try {
        db->executeUpdate(
            "DELETE FROM Employees WHERE email = '" + email + "'");
    } catch (...) {}

    // Add employee via inline command
    ASSERT_NO_THROW(runCommand(
        "add \"firstname:HashTest,lastname:User,email:" + email +
        ",password:" + plaintext + "\""));

    // Query the stored values
    std::string hashInDb = queryString(
        "SELECT password FROM Employees WHERE email = '" + email + "'");
    std::string saltInDb = queryString(
        "SELECT passwordsalt FROM Employees WHERE email = '" + email + "'");

    // Hash must be 64 hex chars
    EXPECT_EQ(hashInDb.size(), 64u);
    // Salt must be 64 hex chars
    EXPECT_EQ(saltInDb.size(), 64u);
    // Neither must contain the plaintext
    EXPECT_EQ(hashInDb.find(plaintext), std::string::npos);
    EXPECT_EQ(saltInDb.find(plaintext), std::string::npos);
}

// ─────────────────────────────────────────────────────────────────────────────
// auth::authenticate — integration tests
//
// These tests require an employee added with a known password so that
// authenticate() can look them up in osoem_test.
// ─────────────────────────────────────────────────────────────────────────────

TEST_F(AuthIntegrationTest, AuthenticateCorrectCredentials) {
    // Add a known employee if not present
    const std::string email = "auth.ok@osoem.test";
    const std::string pass = "correctpass";

    try {
        db->executeUpdate(
            "DELETE FROM Employees WHERE email = '" + email + "'");
    } catch (...) {}

    navigateTo({"Organization", "Departments"});
    commands::select(*db, pth, "1");
    pth.reset();
    navigateTo({"Organization", "Departments"});
    commands::select(*db, pth, "1");

    ASSERT_NO_THROW(runCommand(
        "add \"firstname:Auth,lastname:Ok,email:" + email +
        ",password:" + pass + "\""));

    auth::UserContext ctx;
    bool ok = auth::authenticate(*db, email, pass, ctx);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(ctx.isAuthenticated);
    EXPECT_EQ(ctx.email, email);
}

TEST_F(AuthIntegrationTest, AuthenticateWrongPassword) {
    const std::string email = "auth.ok@osoem.test"; // re-use from previous add
    auth::UserContext ctx;
    bool ok = auth::authenticate(*db, email, "wrongpassword", ctx);
    EXPECT_FALSE(ok);
    EXPECT_FALSE(ctx.isAuthenticated);
}

TEST_F(AuthIntegrationTest, AuthenticateNonExistentEmail) {
    auth::UserContext ctx;
    bool ok = auth::authenticate(*db, "nobody@osoem.test", "anypass", ctx);
    EXPECT_FALSE(ok);
    EXPECT_FALSE(ctx.isAuthenticated);
}
