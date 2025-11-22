#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

namespace utils {
    // Print usage/help information
    void printUsage();

    // Parse date string (YYYY-MM-DD) and validate
    bool isValidDate(const std::string& date);

    // Format date for SQL
    std::string formatDateForSQL(const std::string& date);

    // Split string by delimiter
    std::vector<std::string> split(const std::string& str, char delimiter);

    // Check if string is a valid number
    bool isNumber(const std::string& str);

    // Convert string to integer safely
    int toInt(const std::string& str, int defaultValue = 0);

    // Convert string to decimal safely
    double toDouble(const std::string& str, double defaultValue = 0.0);

    // Get user confirmation (yes/no)
    bool confirmAction(const std::string& message);
}

#endif // UTILS_H
