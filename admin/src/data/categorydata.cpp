#include "categorydata.h"
#include "../exceptions/exceptions.h"
#include "../utils/utils.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace data {


CategoryData::CategoryData(Database& db, int projectId) : db(db) {
    Record temp;
    temp.projectId = projectId;
    temp.name = utils::promptString("CategoryName: ");
    validateRecord(temp);
    records.emplace_back(temp);
}

CategoryData::CategoryData(Database& db, std::vector<std::map<std::string,std::string>> rows, int projectId, bool ignoreDuplicates) : db(db) {
    for (const auto& row : rows) {
        Record temp;
        temp.projectId = projectId;
        temp.name = row.count("categoryname") ? row.at("categoryname") : "";
        try {
            validateRecord(temp);
        } catch (DuplicateEntryError& e) {
            if (!ignoreDuplicates) throw e;
        }
        records.emplace_back(temp);
    }
}

CategoryData::CategoryData( int categoryId, Database* db_ptr) : db(*db_ptr) {
    std::unique_ptr<sql::PreparedStatement> selectStmt;
    selectStmt = db.prepareStatement(
        "SELECT categoryname, projectid FROM ActivityCategories WHERE categoryid = ?"
    );
    selectStmt->setInt(1, categoryId);
    auto res = selectStmt->executeQuery();
    Record temp;
    while (res->next()) {
        temp.id = categoryId;
        temp.name = res->getString("categoryname");
        temp.projectId = res->getInt("projectid");
        records.emplace_back(temp);
    }
}

void CategoryData::setValue(std::string fieldName, std::string fieldValue) {
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
    for (auto& rec : records) {
        if (fieldName == "categoryname") {
            rec.name = fieldValue;
        } else {
            throw InvalidFieldError("Invalid field name. Valid fields are: categoryname.");
        }
        validateRecord(rec);
    }
}

void CategoryData::validate() {
    for (auto& record : records) {
        validateRecord(record);
    }
}

void CategoryData::validateRecord(Record& record) {
    // Validate name is not empty
    if (record.name.empty()) {
        throw ValidationError("Category name cannot be empty");
    }
    if (utils::containsReservedChars(record.name)) {
        throw ValidationError("Category name cannot contain reserved characters (,  :  /  ~)");
    }
    if (utils::startsWithNumber(record.name)) {
        throw ValidationError("Category name cannot start with a number");
    }
    if (utils::startsWithDot(record.name)) {
        throw ValidationError("Category name cannot start with '.'");
    }
    if (!utils::isValidLength(record.name, 255)) {
        throw ValidationError("Category name must not exceed 255 characters");
    }

    // Validate project ID is positive
    if (record.projectId <= 0) {
        throw ValidationError("Project ID must be a positive integer");
    }

    // Check for duplicates
    auto stmt = db.prepareStatement(
        "SELECT categoryid FROM ActivityCategories WHERE projectid = ? AND categoryname = ? AND categoryid <> ?"
    );
    stmt->setInt(1, record.projectId);
    stmt->setString(2, record.name);
    stmt->setInt(3, record.id);
    auto res = stmt->executeQuery();
    if (res->next()) {
        throw DuplicateEntryError("category", record.name);
    }

}
} // namespace data

