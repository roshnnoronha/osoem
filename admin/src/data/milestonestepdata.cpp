#include "milestonestepdata.h"
#include "../exceptions/exceptions.h"
#include "../utils/utils.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace data {

MilestoneStepData::MilestoneStepData(Database& db, int milestoneId) : db(db) {
    Record temp;
    temp.milestoneId = milestoneId;
    temp.name = utils::promptString("MilestoneStepName: ");
    temp.progressRatio = utils::promptDouble("ProgressRatio: ");
    validateRecord(temp);
    records.emplace_back(temp);
}

MilestoneStepData::MilestoneStepData(Database& db, std::vector<std::map<std::string,std::string>> rows, int milestoneId, bool ignoreDuplicates) : db(db) {
    for (const auto& row : rows) {
        Record temp;
        temp.milestoneId = milestoneId;
        temp.name = row.count("milestonestepname") ? row.at("milestonestepname") : "";

        if (row.count("progressratio") && !row.at("progressratio").empty()) {
            temp.progressRatio = std::stod(row.at("progressratio"));
        } else {
            temp.progressRatio = 0.0;
        }
        try {
            validateRecord(temp);
        } catch (DuplicateEntryError& e) {
            if (!ignoreDuplicates) throw e;
        }
        records.emplace_back(temp);
    }
}

MilestoneStepData::MilestoneStepData( int milestoneStepId, Database* db_ptr) : db(*db_ptr) {
    std::unique_ptr<sql::PreparedStatement> selectStmt;
    if (milestoneStepId) {
        selectStmt = db.prepareStatement(
            "SELECT milestoneid, milestonestepname, progressratio FROM MilestoneSteps WHERE milestonestepid = ?"
        );
        selectStmt->setInt(1, milestoneStepId);
    } else {
        selectStmt = db.prepareStatement(
            "SELECT milestoneid, milestonestepname, progressratio FROM MilestoneSteps"
        );
    }
    auto res = selectStmt->executeQuery();
    Record temp;
    while (res->next()) {
        temp.id = milestoneStepId;
        temp.milestoneId = res->getInt("milestoneid");
        temp.name = res->getString("milestonestepname");
        temp.progressRatio = res->getDouble("progressratio");
        records.emplace_back(temp);
    }
}

void MilestoneStepData::setValue(std::string fieldName, std::string fieldValue) {
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
    for (auto& rec : records) {
        if (fieldName == "milestonestepname") {
            rec.name = fieldValue;
        } else if (fieldName == "progressratio") {
            try {
                rec.progressRatio = std::stod(fieldValue);
            } catch (const std::exception& e) {
                throw InvalidFieldError("Invalid value for progressratio. Must be a number.");
            }
        } else {
            throw InvalidFieldError("Invalid field name. Valid fields are: milestonestepname, progressratio.");
        }
        validateRecord(rec);
    }
}

void MilestoneStepData::validate() {
    for (auto& record : records) {
        validateRecord(record);
    }
}

void MilestoneStepData::validateRecord(Record& record) {
    // Validate milestone ID is positive
    if (record.milestoneId <= 0) {
        throw ValidationError("Milestone ID must be a positive integer");
    }

    // Validate name is not empty
    if (record.name.empty()) {
        throw ValidationError("Milestone step name cannot be empty");
    }
    if (utils::containsReservedChars(record.name)) {
        throw ValidationError("Milestone step name cannot contain reserved characters (,  :  /  ~)");
    }
    if (utils::startsWithNumber(record.name)) {
        throw ValidationError("Milestone step name cannot start with a number");
    }
    if (utils::startsWithDot(record.name)) {
        throw ValidationError("Milestone step name cannot start with '.'");
    }
    if (!utils::isValidLength(record.name, 255)) {
        throw ValidationError("Milestone step name must not exceed 255 characters");
    }

    // Validate progress ratio is between 0 and 1
    if (record.progressRatio < 0.0 || record.progressRatio > 1.0) {
        throw ValidationError("Progress ratio must be between 0 and 1");
    }

    // Check for duplicates
    auto stmt = db.prepareStatement(
        "SELECT milestonestepid FROM MilestoneSteps WHERE milestoneid = ? AND milestonestepname = ? AND milestonestepid <> ?"
    );
    stmt->setInt(1, record.milestoneId);
    stmt->setString(2, record.name);
    stmt->setInt(3, record.id);
    auto res = stmt->executeQuery();
    if (res->next()) {
        throw DuplicateEntryError("milestone step", record.name);
    }
}
} // namespace data

