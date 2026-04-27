#include "taskdata.h"
#include "../exceptions/exceptions.h"
#include "../utils/utils.h"
#include "../utils/lookup.h"
#include "../utils/readline_wrapper.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace data {


TaskData::TaskData(Database& db, int activityId, int taskId) : db(db){
    Record temp;
    temp.activityId = activityId;
    temp.parentTaskId = taskId;
    temp.name = utils::promptString("TaskName: ");
    temp.description = utils::promptString("TaskDescription (optional): ");
    std::string deptInput = readline_wrapper::readLineWithCompletion("Department ID or Name (optional): ",
        [&db](const std::string& prefix) {
            return utils::completionDepartments(db, prefix);
        });
    if (deptInput.empty()) {
        temp.departmentId = 0;
    } else {
        temp.departmentId = utils::resolveIdOrName(db, deptInput, utils::lookupDepartment);
    }
    records.emplace_back(temp);
}

TaskData::TaskData(Database& db, std::vector<std::map<std::string,std::string>> rows, int activityId, int taskId, bool ignoreDuplicates) : db(db) {
    for (const auto& row : rows) {
        Record temp;
        temp.activityId = activityId;
        temp.parentTaskId = taskId;
        temp.name = row.count("taskname") ? row.at("taskname") : "";
        temp.description = row.count("taskdescription") ? row.at("taskdescription") : "";

        if (row.count("departmentname") && !row.at("departmentname").empty()) {
            temp.departmentId = utils::resolveIdOrName(db, row.at("departmentname"), utils::lookupDepartment);
        } else if (row.count("departmentid") && !row.at("departmentid").empty()) {
            temp.departmentId = std::stoi(row.at("departmentid"));
        } else {
            temp.departmentId = 0;
        }

        records.emplace_back(temp);
    }
}

TaskData::TaskData( int taskId, Database* db_ptr) : db(*db_ptr) {
    auto selectStmt = db.prepareStatement(
        "SELECT taskname, taskdescription, activityid, parenttaskid, departmentid FROM ActivityTasks WHERE taskid = ?"
    );
    selectStmt->setInt(1, taskId);
    auto res = selectStmt->executeQuery();
    Record temp;
    while (res->next()) {
        temp.id = taskId;
        temp.name = res->getString("taskname");
        temp.description = res->isNull("taskdescription") ? "" : res->getString("taskdescription");
        temp.activityId = res->getInt("activityid");
        temp.parentTaskId = res->isNull("parenttaskid") ? 0 : res->getInt("parenttaskid");
        temp.departmentId = res->isNull("departmentid") ? 0 : res->getInt("departmentid");
        records.emplace_back(temp);
    }
}

void TaskData::setValue(std::string fieldName, std::string fieldValue) {
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
    Record oldVal;
    for (auto& rec : records) {
        try {
            oldVal = rec;
            if (fieldName == "taskname") {
                rec.name = fieldValue;
            } else if (fieldName == "taskdescription" || fieldName == "description") {
                rec.description = fieldValue;
            } else if (fieldName == "departmentid" || fieldName == "departmentname") {
                rec.departmentId = utils::resolveIdOrName(db, fieldValue, utils::lookupDepartment);
            } else {
                throw InvalidFieldError("Invalid field name. Valid fields are: taskname, taskdescription, departmentid.");
            }
            validateRecord (rec);
        } catch (const ValidationError& e) {
            rec = oldVal;
            throw e;
        } 
    }
}

void TaskData::validate() {
    for ( auto& record : records) {
        validateRecord(record);
    }
}

void TaskData::validateRecord(Record& record) {
    // Validate name is not empty
    if (record.name.empty()) {
        throw ValidationError("Task name cannot be empty");
    }
    if (utils::containsReservedChars(record.name)) {
        throw ValidationError("Task name cannot contain reserved characters (,  :  /  ~)");
    }
    if (utils::startsWithNumber(record.name)) {
        throw ValidationError("Task name cannot start with a number");
    }
    if (utils::startsWithDot(record.name)) {
        throw ValidationError("Task name cannot start with '.'");
    }
    if (!utils::isValidLength(record.name, 255)) {
        throw ValidationError("Task name must not exceed 255 characters");
    }

    // Validate description length (when non-empty)
    if (!record.description.empty() && !utils::isValidLength(record.description, 2000)) {
        throw ValidationError("Task description must not exceed 2000 characters");
    }

    // Validate activity ID is positive
    if (record.activityId <= 0) {
        throw ValidationError("Activity ID must be a positive integer");
    }

    // Parent task ID can be 0 (for top-level tasks) or positive
    if (record.parentTaskId < 0) {
        throw ValidationError("Parent task ID must be non-negative");
    }

    // Check for duplicates
    std::unique_ptr<sql::PreparedStatement> stmt;
    if (record.parentTaskId > 0) {
        stmt = db.prepareStatement(
            "SELECT taskid FROM ActivityTasks WHERE activityid = ? AND taskname = ? AND parenttaskid = ? AND taskid <> ?"
        );
        stmt->setInt(1, record.activityId);
        stmt->setString(2, record.name);
        stmt->setInt(3, record.parentTaskId);
        stmt->setInt(4, record.id);
    } else {
        stmt = db.prepareStatement(
            "SELECT taskid FROM ActivityTasks WHERE activityid = ? AND taskname = ? AND parenttaskid IS NULL AND taskid <> ?"
        );
        stmt->setInt(1, record.activityId);
        stmt->setString(2, record.name);
        stmt->setInt(3, record.id);
    }
    auto res = stmt->executeQuery();
    if (res->next()) {
        throw DuplicateEntryError("task", record.name);
    }
}
} // namespace data

