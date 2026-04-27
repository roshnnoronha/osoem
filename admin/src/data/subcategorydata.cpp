#include "subcategorydata.h"
#include "../exceptions/exceptions.h"
#include "../utils/utils.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace data {


SubCategoryData::SubCategoryData(Database& db, int categoryId): db(db){
    Record temp;
    temp.categoryId = categoryId;
    temp.name = utils::promptString("SubCategoryName: ");
    validateRecord(temp);
    records.emplace_back(temp);
}

SubCategoryData::SubCategoryData(Database& db, std::vector<std::map<std::string,std::string>> rows, int categoryId, bool ignoreDuplicates) : db(db) {
    for (const auto& row : rows) {
        Record temp;
        temp.categoryId = categoryId;
        temp.name = row.count("subcategoryname") ? row.at("subcategoryname") : "";
        try {
            validateRecord(temp);
        } catch (DuplicateEntryError& e) {
            if (!ignoreDuplicates) throw e;
        }
        records.emplace_back(temp);
    }
}

SubCategoryData::SubCategoryData( int subcategoryId, Database* db_ptr) : db(*db_ptr) {
    std::unique_ptr<sql::PreparedStatement> selectStmt;
    if (subcategoryId) {
        selectStmt = db.prepareStatement(
            "SELECT subcategoryname, categoryid FROM ActivitySubcategories WHERE subcategoryid = ?"
        );
        selectStmt->setInt(1, subcategoryId);
    } else {
        selectStmt = db.prepareStatement(
            "SELECT subcategoryname, categoryid FROM ActivitySubcategories"
        );
    }
    auto res = selectStmt->executeQuery();
    Record temp;
    while (res->next()) {
        temp.id = subcategoryId;
        temp.name = res->getString("subcategoryname");
        temp.categoryId = res->getInt("categoryid");
        records.emplace_back(temp);
    }
}

void SubCategoryData::setValue(std::string fieldName, std::string fieldValue) {
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
    for (auto& rec : records) {
        if (fieldName == "subcategoryname") {
            rec.name = fieldValue;
        } else {
            throw InvalidFieldError("Invalid field name. Valid fields are: subcategoryname.");
        }
        validateRecord(rec);
    }
}

void SubCategoryData::validate() {
    for (auto& record : records) {
        validateRecord(record);
    }
}

void SubCategoryData::validateRecord(Record& record) {
    // Validate name is not empty
    if (record.name.empty()) {
        throw ValidationError("Sub-category name cannot be empty");
    }
    if (utils::containsReservedChars(record.name)) {
        throw ValidationError("Sub-category name cannot contain reserved characters (,  :  /  ~)");
    }
    if (utils::startsWithNumber(record.name)) {
        throw ValidationError("Sub-category name cannot start with a number");
    }
    if (utils::startsWithDot(record.name)) {
        throw ValidationError("Sub-category name cannot start with '.'");
    }
    if (!utils::isValidLength(record.name, 255)) {
        throw ValidationError("Sub-category name must not exceed 255 characters");
    }

    // Validate category ID is positive
    if (record.categoryId <= 0) {
        throw ValidationError("Category ID must be a positive integer");
    }

    // Check for duplicates
    auto stmt = db.prepareStatement(
        "SELECT subcategoryid FROM ActivitySubcategories WHERE categoryid = ? AND subcategoryname = ? AND subcategoryid <> ? "
    );
    stmt->setInt(1, record.categoryId);
    stmt->setString(2, record.name);
    stmt->setInt(3, record.id);
    auto res = stmt->executeQuery();
    if (res->next()) {
        throw DuplicateEntryError("subcategory", record.name);
    }

}

} // namespace data

