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

    // Check if string length is within limit
    bool isValidLength(const std::string& str, size_t maxLen);

    // Check if string is a valid number
    bool isNumber(const std::string& str);

    // Convert string to integer safely
    int toInt(const std::string& str, int defaultValue = 0);

    // Convert string to decimal safely
    double toDouble(const std::string& str, double defaultValue = 0.0);

    // Get user confirmation (yes/no)
    bool confirmAction(const std::string& message);

    // Convert string to lowercase
    std::string toLower(const std::string& str);

    // Interactive prompt functions (throw UserCancelledError on ESC)
    std::string promptString(const std::string& prompt);
    int promptInt(const std::string& prompt);
    int promptOptionalInt(const std::string& prompt, int defaultValue = 0);
    double promptDouble(const std::string& prompt);
    bool promptBool(const std::string& prompt);

    // Check if string starts with a digit
    bool startsWithNumber(const std::string& str);

    // Check if string starts with a dot
    bool startsWithDot(const std::string& str);

    // Check if string contains reserved characters (,  :  ~  /)
    bool containsReservedChars(const std::string& str);

    // CSV header validation
    // Validates that the CSV header contains the expected field names (case-insensitive)
    // Throws CsvParseError if validation fails
    void validateCsvHeader(const std::string& headerLine,
                          const std::vector<std::string>& expectedFields,
                          const std::vector<std::string>& optionalFields = {});

    // Flexible CSV header field info
    struct CsvFieldInfo {
        std::string headerName;      // actual header found
        std::string canonicalName;   // base field name (e.g., "employee" for both "employeeid" and "employeename")
        bool usesName;               // true if "...name" variant was used
    };

    // Flexible CSV header validation that supports ID/name alternatives
    // idNamePairs: pairs of (idFieldName, nameFieldName) e.g., {{"employeeid", "employeename"}}
    // fixedFields: fields that don't have alternatives
    // Returns vector of CsvFieldInfo for each field in header order
    std::vector<CsvFieldInfo> validateCsvHeaderFlexible(
        const std::string& headerLine,
        const std::vector<std::pair<std::string, std::string>>& idNamePairs,
        const std::vector<std::string>& fixedFields = {});
}

#endif // UTILS_H
