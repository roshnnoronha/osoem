#include "milestonedata.h"
#include "../exceptions/exceptions.h"
#include "../utils/utils.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace data {

MilestoneData::MilestoneData(Database& db, int projectId) : db(db) {
    Record temp;
    temp.projectId = projectId;
    temp.name = utils::promptString("MilestoneName: ");
    validateRecord(temp);
    records.emplace_back(temp);
}

MilestoneData::MilestoneData(Database& db, std::vector<std::map<std::string,std::string>> rows, int projectId, bool ignoreDuplicates) : db(db) {
    for (const auto& row : rows) {
        Record temp;
        temp.projectId = projectId;
        temp.name = row.count("milestonename") ? row.at("milestonename") : "";
        try {
            validateRecord(temp);
        } catch (DuplicateEntryError& e) {
            if (!ignoreDuplicates) throw e; 
        }
        records.emplace_back(temp);
    }
}

MilestoneData::MilestoneData( int milestoneId, Database* db_ptr) : db(*db_ptr) {
    std::unique_ptr<sql::PreparedStatement> selectStmt;
    if (milestoneId) {
        selectStmt = db.prepareStatement(
            "SELECT projectid, milestonename FROM Milestones WHERE milestoneid = ?"
        );
        selectStmt->setInt(1, milestoneId);
    } else {
        selectStmt = db.prepareStatement(
            "SELECT projectid, milestonename FROM Milestones"
        );
    }
    auto res = selectStmt->executeQuery();
    Record temp;
    while (res->next()) {
        temp.id = milestoneId;
        temp.projectId = res->getInt("projectid");
        temp.name = res->getString("milestonename");
        records.emplace_back(temp);
    }
}

void MilestoneData::setValue(std::string fieldName, std::string fieldValue) {
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
    for (auto& rec : records) {
        if (fieldName == "milestonename") {
            rec.name = fieldValue;
        } else {
            throw InvalidFieldError("Invalid field name. Valid fields are: milestonename.");
        }
        validateRecord(rec);
    }
}

void MilestoneData::validate() {
    for (auto& record : records) {
        validateRecord(record);
    }
}

void MilestoneData::validateRecord(Record& record) {
    // Validate project ID is positive
    if (record.projectId <= 0) {
        throw ValidationError("Project ID must be a positive integer");
    }

    // Validate name is not empty
    if (record.name.empty()) {
        throw ValidationError("Milestone name cannot be empty");
    }
    if (utils::containsReservedChars(record.name)) {
        throw ValidationError("Milestone name cannot contain reserved characters (,  :  /  ~)");
    }
    if (utils::startsWithNumber(record.name)) {
        throw ValidationError("Milestone name cannot start with a number");
    }
    if (utils::startsWithDot(record.name)) {
        throw ValidationError("Milestone name cannot start with '.'");
    }
    if (!utils::isValidLength(record.name, 255)) {
        throw ValidationError("Milestone name must not exceed 255 characters");
    }

    // Check for duplicates
    auto stmt = db.prepareStatement(
        "SELECT milestoneid FROM Milestones WHERE projectid = ? AND milestonename = ? AND milestoneid <> ?"
    );
    stmt->setInt(1, record.projectId);
    stmt->setString(2, record.name);
    stmt->setInt(3,record.id);
    auto res = stmt->executeQuery();
    if (res->next()) {
        throw DuplicateEntryError("milestone", record.name);
    }
}

} // namespace data

