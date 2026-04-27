#ifndef AUTH_H
#define AUTH_H

#include <string>
#include "../data/database.h"

using namespace data;

namespace auth {

struct UserContext {
    int employeeId;
    std::string email;
    std::string firstName;
    std::string lastName;
    bool isAdmin;
    bool isAuthenticated;

    UserContext() : employeeId(0), isAdmin(true /*false*/), isAuthenticated(false) {}
};

// Generate a random 32-byte salt (returned as 64-character hex string)
std::string generateSalt();

// Hash a password with the given salt using SHA-256
// Returns the hash as a hex string
std::string hashPassword(const std::string& password, const std::string& salt);

// Authenticate a user by email and password
// Returns true if authentication succeeded, false otherwise
// On success, userContext is populated with user information
bool authenticate(Database& db, const std::string& email,
                  const std::string& password, UserContext& userContext);

// Prompt for password with hidden input
std::string promptPassword(const std::string& prompt);

} // namespace auth

#endif // AUTH_H
