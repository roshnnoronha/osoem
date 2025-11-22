#include "utils.h"
#include <iostream>
#include <sstream>
#include <regex>

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
    std::string response;
    std::cout << message << " (yes/no): ";
    std::getline(std::cin, response);

    // Convert to lowercase for comparison
    for (char& c : response) {
        c = std::tolower(c);
    }

    return (response == "yes" || response == "y");
}

} // namespace utils
