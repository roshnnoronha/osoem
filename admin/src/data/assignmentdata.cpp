#include "assignmentdata.h"
#include "../exceptions/exceptions.h"
#include "../utils/utils.h"
#include "../utils/lookup.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace data {

AssignmentData::AssignmentData(Database& db, int taskId, int projectId) : db(db) {
    Record temp;
    temp.taskId = taskId;
    temp.userId = utils::promptIdOrName(db, "UserId (or Name): ", utils::lookupEmployee,
        [&db, projectId](const std::string& prefix) {
            return utils::completionTeamMembers(db, prefix, projectId);
        });
    temp.assignedDate = utils::promptString("AssignedDate (YYYY-MM-DD): ");
    temp.closeDate = utils::promptString("CloseDate (YYYY-MM-DD, optional): ");
    validateRecord(temp);
    records.emplace_back(temp);
}

AssignmentData::AssignmentData(Database& db, std::vector<std::map<std::string,std::string>> rows, int taskId, bool ignoreDuplicates) : db(db) {
    for (const auto& row : rows) {
        Record temp;
        temp.taskId = taskId;
        if (row.count("username") && !row.at("username").empty() ) {
            temp.userId = utils::resolveIdOrName(db, row.at("username"), utils::lookupEmployee);
        } else if (row.count("userid") && !row.at("userid").empty()) {
            temp.userId = std::stoi(row.at("userid"));
        } else {
            temp.userId = 0;
        }
        temp.assignedDate = row.count("assigneddate") ? row.at("assigneddate") : "";
        temp.closeDate = row.count("closedate") ? row.at("closedate") : "";
        try {
            validateRecord(temp);
        } catch (DuplicateEntryError& e) {
            if (!ignoreDuplicates) throw e;
        }
        records.emplace_back(temp);
    }
}

AssignmentData::AssignmentData( int assignmentId, Database* db_ptr) : db(*db_ptr) {
    std::unique_ptr<sql::PreparedStatement> selectStmt;
    selectStmt = db.prepareStatement(
        "SELECT taskid, userid, assigneddate, closedate FROM ActivityTaskAssignments WHERE assignmentid = ?"
    );
    selectStmt->setInt(1, assignmentId);
    auto res = selectStmt->executeQuery();
    Record temp;
    while (res->next()) {
        temp.id = assignmentId;
        temp.taskId = res->getInt("taskid");
        temp.userId = res->getInt("userid");
        temp.assignedDate = res->isNull("assigneddate") ? "" : res->getString("assigneddate");
        temp.closeDate = res->isNull("closedate") ? "" : res->getString("closedate");
        records.emplace_back(temp);
    }
}

void AssignmentData::setValue(std::string fieldName, std::string fieldValue) {
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
    for (auto& rec : records) {
        if (fieldName == "userid" || fieldName == "username") {
            rec.userId = utils::resolveIdOrName(db, fieldValue, utils::lookupEmployee);
        } else if (fieldName == "assigneddate") {
            rec.assignedDate = fieldValue;
        } else if (fieldName == "closedate") {
            rec.closeDate = fieldValue;
        } else {
            throw InvalidFieldError("Invalid field name. Valid fields are: userid, username, assigneddate, closedate");
        }
        validateRecord(rec);
    }
}

void AssignmentData::validate() {
    for (auto& record : records) {
        validateRecord(record);
    }
}

void AssignmentData::validateRecord(Record& record) {
    if (record.taskId <= 0) {
        throw ValidationError("Task ID must be a positive integer");
    }
    if (record.userId <= 0) {
        throw ValidationError("User ID must be a positive integer");
    }
    // Validate assigned date is not empty
    if (record.assignedDate.empty()) {
        throw ValidationError("Assigned date cannot be empty");
    }
    if (!utils::isValidDate(record.assignedDate)) {
        throw ValidationError("Assigned date must be in YYYY-MM-DD format");
    }
    // Validate close date format (when non-empty)
    if (!record.closeDate.empty() && !utils::isValidDate(record.closeDate)) {
        throw ValidationError("Close date must be in YYYY-MM-DD format");
    }
    // Validate close date >= assigned date (when both provided)
    if (!record.assignedDate.empty() && !record.closeDate.empty() &&
        record.closeDate < record.assignedDate) {
        throw ValidationError("Close date must not be before assigned date");
    }

    // Check assigned date is not after the activity's actual finish date
    {
        auto actStmt = db.prepareStatement(
            "SELECT a.actualfinish FROM Activities a "
            "JOIN ActivityTasks t ON t.activityid = a.activityid "
            "WHERE t.taskid = ?"
        );
        actStmt->setInt(1, record.taskId);
        auto actRes = actStmt->executeQuery();
        if (actRes->next() && !actRes->isNull("actualfinish")) {
            std::string actualFinish = actRes->getString("actualfinish");
            if (record.assignedDate > actualFinish) {
                throw ValidationError(
                    "Assigned date (" + record.assignedDate +
                    ") cannot be after the activity's actual finish date (" + actualFinish + ")"
                );
            }
        }
    }

    // Check for overlapping assignments
    std::string overlapQuery =
        "SELECT assignmentid, assigneddate, closedate "
        "FROM ActivityTaskAssignments "
        "WHERE taskid = ? AND userid = ?";
    if (record.id > 0) {
        overlapQuery += " AND assignmentid <> ?";
    }
    auto stmt = db.prepareStatement(overlapQuery);
    stmt->setInt(1, record.taskId);
    stmt->setInt(2, record.userId);
    if (record.id > 0) {
        stmt->setInt(3, record.id);
    }
    auto res = stmt->executeQuery();

    while (res->next()) {
        std::string existingStart = res->isNull("assigneddate") ? "" : res->getString("assigneddate");
        std::string existingEnd = res->isNull("closedate") ? "" : res->getString("closedate");
        std::string newStart = record.assignedDate;
        std::string newEnd = record.closeDate;

        // Two ranges [s1, e1] and [s2, e2] overlap if s1 <= e2 AND s2 <= e1
        // Empty end date means open-ended (infinity)
        bool s1_le_e2 = newEnd.empty() || existingStart.empty() || existingStart <= newEnd;
        bool s2_le_e1 = existingEnd.empty() || newStart.empty() || newStart <= existingEnd;

        if (s1_le_e2 && s2_le_e1) {
            throw DuplicateEntryError("assignment",
                "overlaps with assignment starting " + (existingStart.empty() ? "(no date)" : existingStart));
        }
    }

}

} // namespace data
