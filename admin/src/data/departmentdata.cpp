#include "departmentdata.h"
#include "../exceptions/exceptions.h"
#include "../utils/utils.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace data {

DepartmentData::DepartmentData(Database& db) : db(db) {
    Record temp;
    temp.name = utils::promptString("DepartmentName: ");
    validateRecord(temp);
    records.emplace_back(temp);
}

DepartmentData::DepartmentData(Database& db, std::vector<std::map<std::string,std::string>> rows, bool ignoreDuplicates) : db(db) {
    for (const auto& row : rows) {
        Record temp;
        temp.name = row.count("departmentname") ? row.at("departmentname") : "";
        try {
            validateRecord(temp);
        } catch (DuplicateEntryError& e) {
            if (!ignoreDuplicates) throw e;
        }
        records.emplace_back(temp);
    }
}

DepartmentData::DepartmentData( int departmentId,Database* db_ptr) : db(*db_ptr) {
    std::unique_ptr<sql::PreparedStatement> selectStmt;
    if (departmentId) {
        selectStmt = db.prepareStatement(
            "SELECT departmentname FROM Departments WHERE departmentid = ?"
        );
        selectStmt->setInt(1, departmentId);
    } else {
        selectStmt = db.prepareStatement(
            "SELECT departmentname FROM Departments"
        );
    }
    auto res = selectStmt->executeQuery();
    Record temp;
    while (res->next()) {
        temp.id = departmentId;
        temp.name = res->getString("departmentname");
        records.emplace_back(temp);
    }
}

void DepartmentData::setValue(std::string fieldName, std::string fieldValue) {
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);

    for (auto& rec : records) {
        if (fieldName == "departmentname" || fieldName == "name") {
            rec.name = fieldValue;
        } else {
            throw InvalidFieldError("Invalid field name. Valid fields are: departmentname");
        }
        validateRecord(rec);
    }
}

void DepartmentData::validate() {
    for (auto& record : records) {
        validateRecord(record);
    }
}

void DepartmentData::validateRecord(Record& record) {
    if (record.name.empty()) {
        throw ValidationError("Department name cannot be empty");
    }
    if (utils::containsReservedChars(record.name)) {
        throw ValidationError("Department name cannot contain reserved characters (,  :  /  ~)");
    }
    if (utils::startsWithNumber(record.name)) {
        throw ValidationError("Department name cannot start with a number");
    }
    if (utils::startsWithDot(record.name)) {
        throw ValidationError("Department name cannot start with '.'");
    }
    if (!utils::isValidLength(record.name, 255)) {
        throw ValidationError("Department name must not exceed 255 characters");
    }
    // Check for duplicates
    auto stmt = db.prepareStatement(
        "SELECT departmentid FROM Departments WHERE departmentname = ? AND departmentid <> ?"
    );
    stmt->setString(1, record.name);
    stmt->setInt(2,record.id);
    auto res = stmt->executeQuery();
    if (res->next()) {
        throw DuplicateEntryError("department", record.name);
    }
}
} // namespace data
