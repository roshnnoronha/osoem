#include "auth.h"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <termios.h>
#include <unistd.h>

namespace auth {

std::string generateSalt() {
    unsigned char salt[32];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        throw std::runtime_error("Failed to generate random salt");
    }

    std::stringstream ss;
    for (int i = 0; i < 32; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)salt[i];
    }
    return ss.str();
}

std::string hashPassword(const std::string& password, const std::string& salt) {
    std::string combined = password + salt;

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(combined.c_str()),
           combined.length(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

bool authenticate(Database& db, const std::string& email,
                  const std::string& password, UserContext& userContext) {
    try {
        auto stmt = db.prepareStatement(
            "SELECT employeeid, firstname, lastname, email, password, passwordsalt, admin "
            "FROM Employees WHERE email = ?"
        );
        stmt->setString(1, email);
        auto res = stmt->executeQuery();

        if (!res->next()) {
            return false;
        }

        std::string storedPassword = res->getString("password");
        std::string storedSalt = res->getString("passwordsalt");

        bool passwordMatch = false;

        // Check if salt is empty (legacy plain text password)
        if (storedSalt.empty()) {
            // Plain text comparison for migration support
            passwordMatch = (password == storedPassword);
        } else {
            // Hash the provided password with stored salt and compare
            std::string hashedInput = hashPassword(password, storedSalt);
            passwordMatch = (hashedInput == storedPassword);
        }

        if (!passwordMatch) {
            return false;
        }

        // Authentication successful - populate user context
        userContext.employeeId = res->getInt("employeeid");
        userContext.email = res->getString("email");
        userContext.firstName = res->getString("firstname");
        userContext.lastName = res->getString("lastname");
        userContext.isAdmin = res->getBoolean("admin");
        userContext.isAuthenticated = true;

        return true;

    } catch (sql::SQLException& e) {
        std::cerr << "Database error during authentication: " << e.what() << std::endl;
        return false;
    }
}

std::string promptPassword(const std::string& prompt) {
    std::cout << prompt;
    std::cout.flush();

    // Disable echo
    struct termios oldSettings, newSettings;
    tcgetattr(STDIN_FILENO, &oldSettings);
    newSettings = oldSettings;
    newSettings.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);

    std::string password;
    std::getline(std::cin, password);

    // Restore echo
    tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
    std::cout << std::endl;

    return password;
}

} // namespace auth
