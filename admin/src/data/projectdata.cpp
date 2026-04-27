#include "projectdata.h"
#include "../exceptions/exceptions.h"
#include "../utils/utils.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace data {

ProjectData::ProjectData(Database& db) : db(db) {
    Record temp;
    temp.name = utils::promptString("ProjectName: ");
    temp.number = utils::promptString("ProjectNo: ");
    validateRecord(temp);
    records.emplace_back(temp);
}

ProjectData::ProjectData(Database& db, std::vector<std::map<std::string,std::string>> rows, bool ignoreDuplicates) : db(db) {
    for (const auto& row : rows) {
        Record temp;
        temp.name = row.count("projectname") ? row.at("projectname") : "";
        temp.number = row.count("projectno") ? row.at("projectno") : "";
        try {
            validateRecord(temp);
        } catch (DuplicateEntryError& e) {
            if (!ignoreDuplicates) throw e;
        }
        records.emplace_back(temp);
    }
}

ProjectData::ProjectData( int projectId, Database* db_ptr) : db(*db_ptr) {
    std::unique_ptr<sql::PreparedStatement> selectStmt;
    if (projectId) {
        selectStmt = db.prepareStatement(
            "SELECT projectname, projectno FROM Projects WHERE projectid = ?"
        );
        selectStmt->setInt(1, projectId);
    } else {
        selectStmt = db.prepareStatement(
            "SELECT projectname, projectno FROM Projects"
        );
    }
    auto res = selectStmt->executeQuery();
    Record temp;
    while (res->next()) {
        temp.id = projectId;
        temp.name = res->getString("projectname");
        temp.number = res->getString("projectno");
        records.emplace_back(temp);
    }
}

void ProjectData::setValue(std::string fieldName, std::string fieldValue) {
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
    for (auto& rec : records) {
        if (fieldName == "projectname") {
            rec.name = fieldValue;
        } else if (fieldName == "projectno") {
            rec.number = fieldValue;
        } else {
            throw InvalidFieldError("Invalid field name. Valid fields are: projectname, projectno.");
        }
        validateRecord(rec);
    }
}

void ProjectData::validate() {
    for (auto& record : records) {
        validateRecord(record);
    }
}

void ProjectData::validateRecord(Record& record) {
    // Validate project name is not empty
    if (record.name.empty()) {
        throw ValidationError("Project name cannot be empty");
    }
    if (utils::containsReservedChars(record.name)) {
        throw ValidationError("Project name cannot contain reserved characters (,  :  /  ~)");
    }
    if (utils::startsWithNumber(record.name)) {
        throw ValidationError("Project name cannot start with a number");
    }
    if (utils::startsWithDot(record.name)) {
        throw ValidationError("Project name cannot start with '.'");
    }
    if (!utils::isValidLength(record.name, 255)) {
        throw ValidationError("Project name must not exceed 255 characters");
    }

    // Validate project number is not empty
    if (record.number.empty()) {
        throw ValidationError("Project number cannot be empty");
    }
    if (utils::containsReservedChars(record.number)) {
        throw ValidationError("Project number cannot contain reserved characters (,  :)");
    }
    if (!utils::isValidLength(record.number, 255)) {
        throw ValidationError("Project number must not exceed 255 characters");
    }

    // Check for duplicates
    auto numStmt = db.prepareStatement(
        "SELECT projectid FROM Projects WHERE projectno = ? AND projectid <> ?"
    );
    numStmt->setString(1, record.number);
    numStmt->setInt(2,record.id);
    auto numRes = numStmt->executeQuery();

    auto nameStmt = db.prepareStatement(
        "SELECT projectid FROM Projects WHERE projectname = ? AND projectid <> ?"
    );
    nameStmt->setString(1, record.name);
    nameStmt->setInt(2,record.id);
    auto nameRes = nameStmt->executeQuery();

    bool dupNumber = numRes->next();
    bool dupName = nameRes->next();

    if (dupNumber || dupName) {
        std::string reason = record.name;
        if (dupNumber && dupName) {
            reason += " (duplicate name and number)";
        } else if (dupNumber) {
            reason += " (duplicate number: " + record.number + ")";
        } else {
            reason += " (duplicate name)";
        }
        throw DuplicateEntryError("project", reason);
    }

}

} // namespace data
