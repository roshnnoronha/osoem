#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include "../database.h"
#include "../path.h"
#include <string>

namespace commands {
namespace employee {
    // List all employees
    void listEmployees(Database& db);

    // Add a new employee
    void addEmployee(Database& db, const std::string& firstname, const std::string& lastname,
                    const std::string& email, const std::string& password);

    // Select an employee
    void selectEmployee(Database& db, Path& pth, int id);

    // Remove an employee
    void removeEmployee(Database& db, int id);

} // namespace employee
} // namespace commands

#endif // EMPLOYEE_H
