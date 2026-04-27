#include "employeedata.h"
#include "../exceptions/exceptions.h"
#include "../utils/utils.h"
#include "../utils/lookup.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace data {

EmployeeData::EmployeeData(Database& db, int departmentId) : db(db){
    Record temp;
    temp.departmentId = departmentId;
    temp.firstName = utils::promptString("FirstName: ");
    temp.lastName = utils::promptString("LastName: ");
    temp.email = utils::promptString("Email: ");
    temp.password = utils::promptString("Password: ");
    temp.passwordSalt = "";
    temp.admin = utils::promptBool("Admin (y/n): ");
    validateRecord(temp);
    records.emplace_back(temp);
}

EmployeeData::EmployeeData(Database& db, std::vector<std::map<std::string,std::string>> rows, int departmentId, bool ignoreDuplicates) : db(db) {
    for (const auto& row : rows) {
        Record temp;
        temp.departmentId = departmentId;
        temp.firstName = row.count("firstname") ? row.at("firstname") : "";
        temp.lastName = row.count("lastname") ? row.at("lastname") : "";
        temp.email = row.count("email") ? row.at("email") : "";
        temp.password = row.count("password") ? row.at("password") : "";
        temp.passwordSalt = row.count("passwordsalt") ? row.at("passwordsalt") : "";

        if (row.count("admin") && !row.at("admin").empty()) {
            std::string val = row.at("admin");
            temp.admin = (val == "1" || val == "true" || val == "yes");
        } else {
            temp.admin = false;
        }
        try {
            validateRecord(temp);
        } catch (DuplicateEntryError& e) {
            if (!ignoreDuplicates) throw e;
        }
        records.emplace_back(temp);
    }
}

EmployeeData::EmployeeData(int employeeId, Database* db_ptr) : db(*db_ptr) {
    std::unique_ptr<sql::PreparedStatement> selectStmt;
    selectStmt = db.prepareStatement(
            "SELECT firstname, lastname, email, password, passwordsalt, admin, departmentid FROM Employees WHERE employeeid = ?"
    );
    selectStmt->setInt(1, employeeId);
    auto res = selectStmt->executeQuery();
    Record temp;
    while (res->next()) {
        temp.id = employeeId;
        temp.departmentId = res->getInt("departmentid");
        temp.firstName = res->getString("firstname");
        temp.lastName = res->getString("lastname");
        temp.email = res->getString("email");
        temp.password = res->getString("password");
        temp.passwordSalt = res->isNull("passwordsalt") ? "" : res->getString("passwordsalt");
        temp.admin = res->isNull("admin") ? false : res->getBoolean("admin");
        records.emplace_back(temp);
    }
}

void EmployeeData::setValue (std::string fieldName, std::string fieldValue) {
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
    Record oldVal;
    for (auto& rec : records ) {
        try {
            oldVal = rec;
            if (fieldName == "firstname"){
                rec.firstName = fieldValue;
            } else if (fieldName == "lastname") {
                rec.lastName = fieldValue;
            } else if (fieldName == "email") {
                rec.email = fieldValue;
            } else if (fieldName == "password") {
                rec.password = fieldValue;
            } else if (fieldName == "passwordsalt") {
                rec.passwordSalt = fieldValue;
            } else if (fieldName == "admin") {
                rec.admin = (fieldValue == "1" || fieldValue == "true" || fieldValue == "yes");
            } else {
                throw InvalidFieldError("Invalid field name. Valid fields are: firstname, lastname, email, password, admin");
            }
            validateRecord (rec);
        } catch (const ValidationError& e) {
            rec = oldVal;
            throw e;
        } 
    }
}

void EmployeeData::validate() {
    for ( auto& record : records) {
        validateRecord(record);
    }
}

void EmployeeData::validateRecord(Record& record) {

        // Validate first name is not empty
        if (record.firstName.empty()) {
            throw ValidationError("First name cannot be empty");
        }
        if (utils::containsReservedChars(record.firstName)) {
            throw ValidationError("First name cannot contain reserved characters (,  :  /  ~)");
        }
        if (utils::startsWithNumber(record.firstName)) {
            throw ValidationError("First name cannot start with a number");
        }
        if (utils::startsWithDot(record.firstName)) {
            throw ValidationError("First name cannot start with '.'");
        }
        if (!utils::isValidLength(record.firstName, 255)) {
            throw ValidationError("First name must not exceed 255 characters");
        }

        // Validate last name is not empty
        if (record.lastName.empty()) {
            throw ValidationError("Last name cannot be empty");
        }
        if (utils::containsReservedChars(record.lastName)) {
            throw ValidationError("Last name cannot contain reserved characters (,  :  /  ~)");
        }
        if (utils::startsWithNumber(record.lastName)) {
            throw ValidationError("Last name cannot start with a number");
        }
        if (utils::startsWithDot(record.lastName)) {
            throw ValidationError("Last name cannot start with '.'");
        }
        if (!utils::isValidLength(record.lastName, 255)) {
            throw ValidationError("Last name must not exceed 255 characters");
        }

        // Validate email is not empty
        if (record.email.empty()) {
            throw ValidationError("Email cannot be empty");
        }
        if (!utils::isValidLength(record.email, 255)) {
            throw ValidationError("Email must not exceed 255 characters");
        }
        if (record.email.find('@') == std::string::npos) {
            throw ValidationError("Email must contain an '@' character");
        }

        // Validate password is not empty
        if (record.password.empty()) {
            throw ValidationError("Password cannot be empty");
        }
        if (!utils::isValidLength(record.password, 255)) {
            throw ValidationError("Password must not exceed 255 characters");
        }
        
        // Check if departmentId contains a valid department ID
        std::unique_ptr<sql::PreparedStatement> selectStmt;
        selectStmt = db.prepareStatement(
                "SELECT departmentid FROM Departments WHERE departmentid = ?"
        );
        selectStmt->setInt(1,record.departmentId);
        auto res = selectStmt->executeQuery();
        if (!res->next()) {
            throw ValidationError("Invalid department ID");
        }

        // Check for duplicates
        auto nameStmt = db.prepareStatement(
            "SELECT employeeid FROM Employees WHERE firstname = ? AND lastname = ? AND employeeid <> ?"
        );
        nameStmt->setString(1, record.firstName);
        nameStmt->setString(2, record.lastName);
        nameStmt->setInt(3,record.id);
        auto dupName = nameStmt->executeQuery();
        if (dupName->next()) {
            throw DuplicateEntryError("employee", record.firstName + " " + record.lastName);
        }
        auto emailStmt = db.prepareStatement(
            "SELECT employeeid FROM Employees WHERE email = ? AND employeeid <> ?"
        );
        emailStmt->setString(1, record.email);
        emailStmt->setInt(2,record.id);
        auto dupEmail = emailStmt->executeQuery();
        if (dupEmail->next()) {
            throw DuplicateEntryError("email", record.email);
        }
}

} // namespace data
