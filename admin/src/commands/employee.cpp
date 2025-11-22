#include "employee.h"
#include <iostream>
#include <iomanip>

namespace commands {
namespace employee {

    void listEmployees(Database& db) {
        std::string query = R"(
            SELECT employeeid, firstname, lastname, email
            FROM Employees
            ORDER BY employeeid
        )";

        auto res = db.executeQuery(query);

        std::cout << "\n" << std::string(90, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(20) << "First Name"
                  << std::setw(20) << "Last Name"
                  << std::setw(40) << "Email"
                  << std::endl;
        std::cout << std::string(90, '-') << std::endl;

        int count = 0;
        while (res->next()) {
            std::cout << std::left << std::setw(5) << res->getInt("employeeid")
                      << std::setw(20) << res->getString("firstname").substr(0, 19)
                      << std::setw(20) << res->getString("lastname").substr(0, 19)
                      << std::setw(40) << res->getString("email").substr(0, 39)
                      << std::endl;
            count++;
        }

        std::cout << std::string(90, '=') << std::endl;
        std::cout << "Total: " << count << " employees" << std::endl;
    }

    void addEmployee(Database& db, const std::string& firstname, const std::string& lastname,
                    const std::string& email, const std::string& password) {
        try {
            // Insert employee
            auto insertStmt = db.prepareStatement(
                "INSERT INTO Employees (firstname, lastname, email, password) VALUES (?, ?, ?, ?)"
            );
            insertStmt->setString(1, firstname);
            insertStmt->setString(2, lastname);
            insertStmt->setString(3, email);
            insertStmt->setString(4, password);

            insertStmt->executeUpdate();

            auto lastIdRes = db.executeQuery("SELECT LAST_INSERT_ID() as id");
            if (lastIdRes->next()) {
                std::cout << "Employee created successfully with ID: " << lastIdRes->getInt("id") << std::endl;
            }
        } catch (sql::SQLException& e) {
            std::cerr << "Error creating employee: " << e.what() << std::endl;
            throw;
        }
    }

    void selectEmployee(Database& db, Path& pth, int id) {
        std::string sql = "SELECT CONCAT(firstname, ' ', lastname) as name FROM Employees WHERE employeeid = ?";
        auto stmt = db.prepareStatement(sql);
        stmt->setInt(1, id);
        auto res = stmt->executeQuery();

        if (res->next()) {
            std::string name = res->getString("name");
            pth.move_forward(name, id, Path::EMPLOYEES);
        } else {
            std::cerr << "Invalid employee ID." << std::endl;
        }
    }

    void removeEmployee(Database& db, int id) {
        try {
            db.beginTransaction();

            // Check if employee exists
            auto checkStmt = db.prepareStatement("SELECT firstname, lastname FROM Employees WHERE employeeid = ?");
            checkStmt->setInt(1, id);
            auto res = checkStmt->executeQuery();

            if (!res->next()) {
                std::cerr << "Employee with ID " << id << " not found." << std::endl;
                db.rollback();
                return;
            }

            std::string name = res->getString("firstname") + " " + res->getString("lastname");

            // Check if employee is a project manager
            auto pmCheckStmt = db.prepareStatement(
                "SELECT COUNT(*) as count FROM Projects WHERE projectmanagerid = ?"
            );
            pmCheckStmt->setInt(1, id);
            auto pmRes = pmCheckStmt->executeQuery();
            if (pmRes->next() && pmRes->getInt("count") > 0) {
                std::cerr << "Error: Cannot delete employee " << name
                          << ". They are assigned as project manager." << std::endl;
                db.rollback();
                return;
            }

            // Check if employee is an activity manager
            auto amCheckStmt = db.prepareStatement(
                "SELECT COUNT(*) as count FROM Activities WHERE activitymanagerid = ?"
            );
            amCheckStmt->setInt(1, id);
            auto amRes = amCheckStmt->executeQuery();
            if (amRes->next() && amRes->getInt("count") > 0) {
                std::cerr << "Error: Cannot delete employee " << name
                          << ". They are assigned as activity manager." << std::endl;
                db.rollback();
                return;
            }

            // Delete employee
            auto deleteStmt = db.prepareStatement("DELETE FROM Employees WHERE employeeid = ?");
            deleteStmt->setInt(1, id);
            deleteStmt->executeUpdate();

            db.commit();
            std::cout << "Employee " << name << " deleted successfully." << std::endl;

        } catch (sql::SQLException& e) {
            std::cerr << "Error deleting employee: " << e.what() << std::endl;
            db.rollback();
            throw;
        }
    }

} // namespace employee
} // namespace commands
