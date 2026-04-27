#include "utils.h"
#include "../exceptions/exceptions.h"
#include <iostream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>

namespace utils {

void printUsage() {
    std::cout << "\n=== OSOEM Admin Tool ===\n" << std::endl;
    std::cout << "Usage: osoem_admin <command> [options]\n" << std::endl;
    std::cout << "Commands:\n" << std::endl;

    std::cout << "Activity Management:" << std::endl;
    std::cout << "  import-activities <csv_file> <project_id>" << std::endl;
    std::cout << "      Import activities from CSV file" << std::endl;
    std::cout << "  list-activities [project_id]" << std::endl;
    std::cout << "      List all activities (optionally filtered by project)" << std::endl;
    std::cout << "  add-activity <name> <subcategory_id> <manager_id> <start> <end> <hours>" << std::endl;
    std::cout << "      Add a new activity manually" << std::endl;
    std::cout << "  remove-activity <activity_id>" << std::endl;
    std::cout << "      Remove an activity by ID\n" << std::endl;

    std::cout << "Task Management:" << std::endl;
    std::cout << "  add-task <activity_id> <task_name> [parent_task_id]" << std::endl;
    std::cout << "      Add a task to an activity" << std::endl;
    std::cout << "  list-tasks <activity_id>" << std::endl;
    std::cout << "      List all tasks for an activity\n" << std::endl;

    std::cout << "Artefact Management:" << std::endl;
    std::cout << "  create-artefact-type <project_id> <name> <description>" << std::endl;
    std::cout << "      Create a new artefact type" << std::endl;
    std::cout << "  import-artefacts <csv_file> <artefact_type_id>" << std::endl;
    std::cout << "      Import artefacts from CSV file" << std::endl;
    std::cout << "  list-artefact-types [project_id]" << std::endl;
    std::cout << "      List all artefact types\n" << std::endl;

    std::cout << "Employee Management:" << std::endl;
    std::cout << "  add-employee <firstname> <lastname> <email> <password>" << std::endl;
    std::cout << "      Add a new employee" << std::endl;
    std::cout << "  remove-employee <employee_id>" << std::endl;
    std::cout << "      Remove an employee" << std::endl;
    std::cout << "  list-employees" << std::endl;
    std::cout << "      List all employees\n" << std::endl;

    std::cout << "Milestone Management:" << std::endl;
    std::cout << "  add-milestone <project_id> <name>" << std::endl;
    std::cout << "      Add a new milestone" << std::endl;
    std::cout << "  add-milestone-step <milestone_id> <step_name> <progress_ratio>" << std::endl;
    std::cout << "      Add a step to a milestone" << std::endl;
    std::cout << "  remove-milestone <milestone_id>" << std::endl;
    std::cout << "      Remove a milestone" << std::endl;
    std::cout << "  list-milestones [project_id]" << std::endl;
    std::cout << "      List all milestones\n" << std::endl;

    std::cout << "Linking:" << std::endl;
    std::cout << "  link-artefact <artefact_id> <activity_id> <ratio>" << std::endl;
    std::cout << "      Link an artefact to an activity" << std::endl;
    std::cout << "  link-milestone <milestone_step_id> <artefact_link_id>" << std::endl;
    std::cout << "      Link a milestone step to an artefact-activity link\n" << std::endl;
}

bool isValidDate(const std::string& date) {
    // Check format YYYY-MM-DD
    std::regex datePattern(R"(\d{4}-\d{2}-\d{2})");
    return std::regex_match(date, datePattern);
}

std::string formatDateForSQL(const std::string& date) {
    if (isValidDate(date)) {
        return "'" + date + "'";
    }
    return "NULL";
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

bool isValidLength(const std::string& str, size_t maxLen) {
    return str.length() <= maxLen;
}

bool isNumber(const std::string& str) {
    if (str.empty()) return false;

    size_t start = 0;
    if (str[0] == '-' || str[0] == '+') {
        start = 1;
        if (str.length() == 1) return false;
    }

    bool hasDot = false;
    for (size_t i = start; i < str.length(); i++) {
        if (str[i] == '.') {
            if (hasDot) return false;
            hasDot = true;
        } else if (!std::isdigit(str[i])) {
            return false;
        }
    }

    return true;
}

int toInt(const std::string& str, int defaultValue) {
    try {
        return std::stoi(str);
    } catch (...) {
        return defaultValue;
    }
}

double toDouble(const std::string& str, double defaultValue) {
    try {
        return std::stod(str);
    } catch (...) {
        return defaultValue;
    }
}

bool confirmAction(const std::string& message) {
    std::string response = promptString(message + " (yes/no): ");

    // Convert to lowercase for comparison
    for (char& c : response) {
        c = std::tolower(c);
    }

    return (response == "yes" || response == "y");
}

std::string promptString(const std::string& prompt) {
    std::cout << prompt;
    std::cout.flush();

    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN] = 1;
    newt.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::string result;
    char c;

    while (true) {
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n <= 0) {
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            throw UserCancelledError();
        }

        if (c == 27) { // ESC
            // Check if more chars follow (escape sequence) with 50ms timeout
            fd_set fds;
            struct timeval tv;
            FD_ZERO(&fds);
            FD_SET(STDIN_FILENO, &fds);
            tv.tv_sec = 0;
            tv.tv_usec = 50000; // 50ms

            if (select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0) {
                // Escape sequence (arrow keys, etc.) - read and discard
                char seq[8];
                read(STDIN_FILENO, seq, sizeof(seq));
            } else {
                // Standalone ESC - cancel
                std::cout << std::endl;
                tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
                throw UserCancelledError();
            }
        } else if (c == '\n' || c == '\r') {
            std::cout << std::endl;
            break;
        } else if (c == 127 || c == 8) { // Backspace
            if (!result.empty()) {
                result.pop_back();
                std::cout << "\b \b";
                std::cout.flush();
            }
        } else if (c == 4) { // Ctrl-D
            std::cout << std::endl;
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            throw UserCancelledError();
        } else if (c >= 32) { // Printable
            result += c;
            std::cout << c;
            std::cout.flush();
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return result;
}

int promptInt(const std::string& prompt) {
    std::string input = promptString(prompt);
    try {
        return std::stoi(input);
    } catch (...) {
        throw ValidationError("Invalid integer value: " + input);
    }
}

int promptOptionalInt(const std::string& prompt, int defaultValue) {
    std::string input = promptString(prompt);
    if (input.empty()) {
        return defaultValue;
    }
    try {
        return std::stoi(input);
    } catch (...) {
        throw ValidationError("Invalid integer value: " + input);
    }
}

double promptDouble(const std::string& prompt) {
    std::string input = promptString(prompt);
    try {
        return std::stod(input);
    } catch (...) {
        throw ValidationError("Invalid numeric value: " + input);
    }
}

bool promptBool(const std::string& prompt) {
    std::string input = promptString(prompt);
    std::string lower = toLower(input);
    return (lower == "y" || lower == "yes" || lower == "true" || lower == "1");
}

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

bool startsWithNumber(const std::string& str) {
    return !str.empty() && std::isdigit(static_cast<unsigned char>(str[0]));
}

bool startsWithDot(const std::string& str) {
    return !str.empty() && str[0] == '.';
}

bool containsReservedChars(const std::string& str) {
    return str.find(',') != std::string::npos ||
           str.find(':') != std::string::npos ||
           str.find('~') != std::string::npos ||
           str.find('/') != std::string::npos;
}

void validateCsvHeader(const std::string& headerLine,
                      const std::vector<std::string>& expectedFields,
                      const std::vector<std::string>& optionalFields) {
    // Parse header fields
    std::vector<std::string> headerFields = split(headerLine, ',');

    // Trim whitespace and convert to lowercase for comparison
    for (auto& field : headerFields) {
        // Trim leading/trailing whitespace
        size_t start = field.find_first_not_of(" \t\r\n");
        size_t end = field.find_last_not_of(" \t\r\n");
        if (start != std::string::npos && end != std::string::npos) {
            field = field.substr(start, end - start + 1);
        } else {
            field = "";
        }
        field = toLower(field);
    }

    // Check that required fields are present
    size_t requiredCount = expectedFields.size();
    if (headerFields.size() < requiredCount) {
        std::string expectedList;
        for (size_t i = 0; i < expectedFields.size(); ++i) {
            if (i > 0) expectedList += ", ";
            expectedList += expectedFields[i];
        }
        throw CsvParseError("Invalid CSV header. Expected fields: " + expectedList);
    }

    // Validate each required field matches
    for (size_t i = 0; i < requiredCount; ++i) {
        std::string expectedLower = toLower(expectedFields[i]);
        if (headerFields[i] != expectedLower) {
            throw CsvParseError("Invalid CSV header. Expected '" + expectedFields[i] +
                              "' but found '" + headerFields[i] + "' at position " + std::to_string(i + 1));
        }
    }

    // Validate optional fields if present
    for (size_t i = requiredCount; i < headerFields.size() && (i - requiredCount) < optionalFields.size(); ++i) {
        std::string expectedLower = toLower(optionalFields[i - requiredCount]);
        if (!headerFields[i].empty() && headerFields[i] != expectedLower) {
            throw CsvParseError("Invalid CSV header. Expected '" + optionalFields[i - requiredCount] +
                              "' but found '" + headerFields[i] + "' at position " + std::to_string(i + 1));
        }
    }
}

std::vector<CsvFieldInfo> validateCsvHeaderFlexible(
    const std::string& headerLine,
    const std::vector<std::pair<std::string, std::string>>& idNamePairs,
    const std::vector<std::string>& fixedFields) {

    // Parse header fields
    std::vector<std::string> headerFields = split(headerLine, ',');

    // Trim whitespace and convert to lowercase for comparison
    for (auto& field : headerFields) {
        size_t start = field.find_first_not_of(" \t\r\n");
        size_t end = field.find_last_not_of(" \t\r\n");
        if (start != std::string::npos && end != std::string::npos) {
            field = field.substr(start, end - start + 1);
        } else {
            field = "";
        }
        field = toLower(field);
    }

    // Build expected field count
    size_t expectedCount = idNamePairs.size() + fixedFields.size();
    if (headerFields.size() < expectedCount) {
        std::string expectedList;
        for (size_t i = 0; i < idNamePairs.size(); ++i) {
            if (i > 0) expectedList += ", ";
            expectedList += idNamePairs[i].first + "/" + idNamePairs[i].second;
        }
        for (size_t i = 0; i < fixedFields.size(); ++i) {
            if (!expectedList.empty()) expectedList += ", ";
            expectedList += fixedFields[i];
        }
        throw CsvParseError("Invalid CSV header. Expected fields: " + expectedList);
    }

    std::vector<CsvFieldInfo> result;

    // Process ID/name pairs first
    for (size_t i = 0; i < idNamePairs.size(); ++i) {
        if (i >= headerFields.size()) {
            throw CsvParseError("Missing header field at position " + std::to_string(i + 1));
        }

        CsvFieldInfo info;
        info.headerName = headerFields[i];

        std::string idLower = toLower(idNamePairs[i].first);
        std::string nameLower = toLower(idNamePairs[i].second);

        if (headerFields[i] == idLower) {
            info.usesName = false;
            // Extract canonical name by removing "id" suffix
            info.canonicalName = idNamePairs[i].first.substr(0, idNamePairs[i].first.length() - 2);
        } else if (headerFields[i] == nameLower) {
            info.usesName = true;
            // Extract canonical name by removing "name" suffix
            info.canonicalName = idNamePairs[i].second.substr(0, idNamePairs[i].second.length() - 4);
        } else {
            throw CsvParseError("Invalid CSV header. Expected '" + idNamePairs[i].first +
                              "' or '" + idNamePairs[i].second +
                              "' but found '" + headerFields[i] + "' at position " + std::to_string(i + 1));
        }

        result.push_back(info);
    }

    // Process fixed fields
    for (size_t i = 0; i < fixedFields.size(); ++i) {
        size_t headerIdx = idNamePairs.size() + i;
        if (headerIdx >= headerFields.size()) {
            throw CsvParseError("Missing header field at position " + std::to_string(headerIdx + 1));
        }

        std::string expectedLower = toLower(fixedFields[i]);
        if (headerFields[headerIdx] != expectedLower) {
            throw CsvParseError("Invalid CSV header. Expected '" + fixedFields[i] +
                              "' but found '" + headerFields[headerIdx] + "' at position " + std::to_string(headerIdx + 1));
        }

        CsvFieldInfo info;
        info.headerName = headerFields[headerIdx];
        info.canonicalName = fixedFields[i];
        info.usesName = false;
        result.push_back(info);
    }

    return result;
}

} // namespace utils
