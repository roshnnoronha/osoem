#include "activitydata.h"
#include "../exceptions/exceptions.h"
#include "../utils/utils.h"
#include "../utils/lookup.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace data {


ActivityData::ActivityData(Database& db, int subCategoryId, int projectId) : db(db){
    Record temp;
    temp.subCategoryId = subCategoryId;
    temp.name = utils::promptString("ActivityName: ");
    temp.description = utils::promptString("ActivityDescription: ");
    temp.managerId = utils::promptIdOrName(db, "ActivityManagerId (or Name): ", utils::lookupEmployee,
        [&db, projectId](const std::string& prefix) {
            return utils::completionTeamMembers(db, prefix, projectId);
        });
    temp.plannedStart = utils::promptString("PlannedStart (YYYY-MM-DD, optional): ");
    temp.plannedEnd = utils::promptString("PlannedFinish (YYYY-MM-DD, optional): ");
    temp.forecastStart = utils::promptString("ForecastStart (YYYY-MM-DD, optional): ");
    temp.forecastEnd = utils::promptString("ForecastFinish (YYYY-MM-DD, optional): ");
    temp.actualEnd = utils::promptString("ActualFinish (YYYY-MM-DD, optional): ");
    std::string hoursInput = utils::promptString("PlannedHours (optional): ");
    if (hoursInput.empty()) {
        temp.plannedHours = -1;  // -1 indicates NULL
    } else {
        try {
            temp.plannedHours = std::stod(hoursInput);
        } catch (const std::exception&) {
            throw ValidationError("Invalid value for planned hours. Must be a number.");
        }
    }
    hoursInput = utils::promptString("ForecastHours (optional): ");
    if (hoursInput.empty()) {
        temp.forecastHours = -1;  // -1 indicates NULL
    } else {
        try {
            temp.forecastHours = std::stod(hoursInput);
        } catch (const std::exception&) {
            throw ValidationError("Invalid value for forecast hours. Must be a number.");
        }
    }
    validateRecord(temp);
    records.emplace_back(temp);
}

ActivityData::ActivityData(Database& db, std::vector<std::map<std::string,std::string>> rows, int subCategoryId, bool ignoreDuplicates) : db(db) {
    for (const auto& row : rows) {
        Record temp;
        temp.subCategoryId = subCategoryId;
        temp.name = row.count("activityname") ? row.at("activityname") : "";
        temp.description = row.count("activitydescription") ? row.at("activitydescription") : "";

        if (row.count("activitymanagername") && !row.at("activitymanagername").empty() ) {
            temp.managerId = utils::resolveIdOrName(db, row.at("activitymanagername"), utils::lookupEmployee);
        } else if (row.count("activitymanagerid") && !row.at("activitymanagerid").empty()) {
            temp.managerId = std::stoi(row.at("activitymanagerid"));
        } else {
            throw CsvParseError("ActivityManagerID or ActivityManagerName is a required field for an activity.");
        }

        temp.plannedStart = row.count("plannedstart") ? row.at("plannedstart") : "";
        temp.plannedEnd = row.count("plannedfinish") ? row.at("plannedfinish") : "";
        temp.forecastStart = row.count("forecaststart") ? row.at("forecaststart") : "";
        temp.forecastEnd = row.count("forecastfinish") ? row.at("forecastfinish") : "";
        temp.actualEnd = row.count("actualfinish") ? row.at("actualfinish") : "";

        if (row.count("plannedhours") && !row.at("plannedhours").empty()) {
            temp.plannedHours = std::stod(row.at("plannedhours"));
        } else {
            temp.plannedHours = -1;
        }
        if (row.count("forecasthours") && !row.at("forecasthours").empty()) {
            temp.forecastHours = std::stod(row.at("forecasthours"));
        } else {
            temp.forecastHours = -1;
        }
        try {
            validateRecord(temp);
        } catch (DuplicateEntryError& e) {
            if (!ignoreDuplicates) throw e;
        }
        records.emplace_back(temp);
    }
}

ActivityData::ActivityData(int activityId, Database* db_ptr): db(*db_ptr) {
    std::unique_ptr<sql::PreparedStatement> selectStmt;
    selectStmt = db.prepareStatement(
        "SELECT activityname, activitydescription, activitymanagerid, plannedstart, plannedfinish, forecaststart, forecastfinish, actualfinish, plannedhours, forecasthours, subcategoryid FROM Activities WHERE activityid = ?"
    );
    selectStmt->setInt(1, activityId);
    auto res = selectStmt->executeQuery();
    Record temp;
    while (res->next()) {
        temp.id = activityId;
        temp.subCategoryId = res->getInt("subcategoryid");
        temp.name = res->getString("activityname");
        temp.description = res->getString("activitydescription");
        temp.managerId = res->getInt("activitymanagerid");
        temp.plannedStart = res->getString("plannedstart");
        temp.plannedEnd = res->getString("plannedfinish");
        temp.forecastStart = res->getString("forecaststart");
        temp.forecastEnd = res->getString("forecastfinish");
        temp.actualEnd = res->getString("actualfinish");
        temp.plannedHours = res->getDouble("plannedhours");
        temp.forecastHours = res->getDouble("forecasthours");
        records.emplace_back(temp);
    }
}

void ActivityData::setValue(std::string fieldName, std::string fieldValue) {
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
    for (auto& rec : records) {
        if (fieldName == "activityname") {
            rec.name = fieldValue;
        } else if (fieldName == "activitydescription") {
            rec.description = fieldValue;
        } else if (fieldName == "activitymanagerid" || fieldName == "activitymanagername") {
            int newManagerId = utils::resolveIdOrName(db, fieldValue, utils::lookupEmployee);
            auto teamStmt = db.prepareStatement(
                "SELECT COUNT(*) AS cnt FROM ProjectTeamMembers ptm "
                "JOIN ActivityCategories c ON ptm.projectid = c.projectid "
                "JOIN ActivitySubcategories s ON c.categoryid = s.categoryid "
                "WHERE s.subcategoryid = ? AND ptm.employeeid = ?"
            );
            teamStmt->setInt(1, rec.subCategoryId);
            teamStmt->setInt(2, newManagerId);
            auto teamRes = teamStmt->executeQuery();
            if (teamRes->next() && teamRes->getInt("cnt") == 0) {
                throw ValidationError("Activity manager must be a member of the project team");
            }
            rec.managerId = newManagerId;
        } else if (fieldName == "plannedstart") {
            rec.plannedStart = fieldValue;
        } else if (fieldName == "plannedfinish") {
            rec.plannedEnd = fieldValue;
        } else if (fieldName == "forecaststart") {
            rec.forecastStart = fieldValue;
        } else if (fieldName == "forecastfinish") {
            rec.forecastEnd = fieldValue;
        } else if (fieldName == "actualfinish") {
            rec.actualEnd = fieldValue;
        } else if (fieldName == "plannedhours") {
            if (fieldValue.empty()) {
                rec.plannedHours = -1;  // -1 indicates NULL
            } else {
                try {
                    rec.plannedHours = std::stod(fieldValue);
                } catch (const std::exception& e) {
                    throw InvalidFieldError("Invalid value for plannedhours. Must be a number.");
                }
            }
        } else if (fieldName == "forecasthours") {
            if (fieldValue.empty()) {
                rec.forecastHours = -1;  // -1 indicates NULL
            } else {
                try {
                    rec.forecastHours = std::stod(fieldValue);
                } catch (const std::exception& e) {
                    throw InvalidFieldError("Invalid value for forecasthours. Must be a number.");
                }
            }
        } else {
            throw InvalidFieldError("Invalid field name. Valid fields are: activityname, activitydescription, activitymanagerid, activitymanagername, plannedstart, plannedfinish, forecaststart, forecastfinish, actualfinish, plannedhours, forecasthours.");
        }
    }
}

void ActivityData::validate() {
    for (auto& record : records) {
        validateRecord(record);
    }
}

void ActivityData::validateRecord(Record& record) {
    // Validate activity name is not empty
    if (record.name.empty()) {
        throw ValidationError("Activity name cannot be empty");
    }
    if (utils::containsReservedChars(record.name)) {
        throw ValidationError("Activity name cannot contain reserved characters (,  :  /  ~)");
    }
    if (utils::startsWithNumber(record.name)) {
        throw ValidationError("Activity name cannot start with a number");
    }
    if (utils::startsWithDot(record.name)) {
        throw ValidationError("Activity name cannot start with '.'");
    }
    if (!utils::isValidLength(record.name, 255)) {
        throw ValidationError("Activity name must not exceed 255 characters");
    }

    // Validate activity desctiption
    if (utils::containsReservedChars(record.description)) {
        throw ValidationError("Activity description cannot contain reserved characters (,  :)");
    }
    if (utils::startsWithNumber(record.description)) {
        throw ValidationError("Activity description cannot start with a number");
    }
    if (!utils::isValidLength(record.description, 1000)) {
        throw ValidationError("Activity description must not exceed 1000 characters");
    }

    // Validate sub-category ID is positive
    if (record.subCategoryId <= 0) {
        throw ValidationError("Sub-category ID must be a positive integer");
    }

    // Validate manager ID is positive
    if (record.managerId <= 0) {
        throw ValidationError("Manager ID must be a positive integer");
    }

    // Validate manager is a member of the project team
    {
        auto teamStmt = db.prepareStatement(
            "SELECT COUNT(*) AS cnt FROM ProjectTeamMembers ptm "
            "JOIN ActivityCategories c ON ptm.projectid = c.projectid "
            "JOIN ActivitySubcategories s ON c.categoryid = s.categoryid "
            "WHERE s.subcategoryid = ? AND ptm.employeeid = ?"
        );
        teamStmt->setInt(1, record.subCategoryId);
        teamStmt->setInt(2, record.managerId);
        auto teamRes = teamStmt->executeQuery();
        if (teamRes->next() && teamRes->getInt("cnt") == 0) {
            throw ValidationError("Activity manager must be a member of the project team");
        }
    }

    // Validate planned start date format (when non-empty)
    if (!record.plannedStart.empty() && !utils::isValidDate(record.plannedStart)) {
        throw ValidationError("Planned start date must be in YYYY-MM-DD format");
    }

    // Validate planned end date format (when non-empty)
    if (!record.plannedEnd.empty() && !utils::isValidDate(record.plannedEnd)) {
        throw ValidationError("Planned end date must be in YYYY-MM-DD format");
    }

    // Validate planned start <= planned end (when both provided)
    if (!record.plannedStart.empty() && !record.plannedEnd.empty() &&
        record.plannedStart > record.plannedEnd) {
        throw ValidationError("Planned start date must not be after planned end date");
    }
    //
    // Validate forecast start date format (when non-empty)
    if (!record.forecastStart.empty() && !utils::isValidDate(record.forecastStart)) {
        throw ValidationError("Forecast start date must be in YYYY-MM-DD format");
    }

    // Validate forecast end date format (when non-empty)
    if (!record.forecastEnd.empty() && !utils::isValidDate(record.forecastEnd)) {
        throw ValidationError("Forecast end date must be in YYYY-MM-DD format");
    }

    // Validate forecast start <= forecast end (when both provided)
    if (!record.forecastStart.empty() && !record.forecastEnd.empty() &&
        record.forecastStart > record.forecastEnd) {
        throw ValidationError("Forecast start date must not be after forecast end date");
    }

    // Validate actual end date format (when non-empty)
    if (!record.actualEnd.empty() && !utils::isValidDate(record.actualEnd)) {
        throw ValidationError("Actual end date must be in YYYY-MM-DD format");
    }

    // Validate actual end date is after the actual start (derived from first booked hours)
    if (!record.actualEnd.empty() && record.id > 0) {
        auto hoursStmt = db.prepareStatement(
            "SELECT MIN(h.bookeddate) AS firstbooked "
            "FROM Hours h "
            "JOIN ActivityTaskAssignments a ON a.assignmentid = h.assignmentid "
            "JOIN ActivityTasks t ON t.taskid = a.taskid "
            "WHERE t.activityid = ?"
        );
        hoursStmt->setInt(1, record.id);
        auto hoursRes = hoursStmt->executeQuery();
        if (hoursRes->next() && !hoursRes->isNull("firstbooked")) {
            std::string firstBooked = hoursRes->getString("firstbooked");
            if (record.actualEnd < firstBooked) {
                throw ValidationError(
                    "Actual finish date (" + record.actualEnd +
                    ") cannot be before the first booked date (" + firstBooked + ")"
                );
            }
        }
    }
    // Validate no hours are booked after the actual end date
    if (!record.actualEnd.empty() && record.id > 0) {
        auto lateStmt = db.prepareStatement(
            "SELECT MAX(h.bookeddate) AS lastbooked "
            "FROM Hours h "
            "JOIN ActivityTaskAssignments a ON a.assignmentid = h.assignmentid "
            "JOIN ActivityTasks t ON t.taskid = a.taskid "
            "WHERE t.activityid = ?"
        );
        lateStmt->setInt(1, record.id);
        auto lateRes = lateStmt->executeQuery();
        if (lateRes->next() && !lateRes->isNull("lastbooked")) {
            std::string lastBooked = lateRes->getString("lastbooked");
            if (lastBooked > record.actualEnd) {
                throw ValidationError(
                    "Hours have been booked after the actual finish date (" + record.actualEnd +
                    "). Last booked date: " + lastBooked
                );
            }
        }
    }

    // Validate planned hours is non-negative (or -1 for NULL)
    if (record.plannedHours < -1) {
        throw ValidationError("Planned hours must be non-negative");
    }
    if (record.plannedHours > 99999999.99) {
        throw ValidationError("Planned hours must not exceed 99999999.99");
    }

    // Validate forecast hours is non-negative (or -1 for NULL)
    if (record.forecastHours < -1) {
        throw ValidationError("Forecast hours must be non-negative");
    }
    if (record.forecastHours > 99999999.99) {
        throw ValidationError("Forecast hours must not exceed 99999999.99");
    }

    // Check for duplicates
    auto stmt = db.prepareStatement(
        "SELECT activityid FROM Activities WHERE subcategoryid = ? AND activityname = ? AND activityid <> ?"
    );
    stmt->setInt(1, record.subCategoryId);
    stmt->setString(2, record.name);
    stmt->setInt(3, record.id);
    auto res = stmt->executeQuery();
    if (res->next()) {
        throw DuplicateEntryError("activity", record.name);
    }

}
} // namespace data

