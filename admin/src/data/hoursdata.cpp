#include "hoursdata.h"
#include "../exceptions/exceptions.h"
#include "../utils/utils.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace data {

HoursData::HoursData(Database& db, int assignmentId) : db(db) {
    Record temp;
    temp.assignmentId = assignmentId;
    temp.bookedDate = utils::promptString("BookedDate (YYYY-MM-DD): ");
    temp.hours = utils::promptInt("Hours: ");
    validateRecord(temp);
    records.emplace_back(temp);
}

HoursData::HoursData(Database &db, std::vector<std::map<std::string,std::string>> rows, int assignmentId, bool ignoreDuplicates) : db(db) {
    for (const auto& row : rows) {
        Record temp;
        temp.assignmentId = assignmentId;
        temp.bookedDate = row.count("bookeddate") ? row.at("bookeddate") : "";

        if (row.count("hours") && !row.at("hours").empty()) {
            temp.hours = std::stoi(row.at("hours"));
        } else {
            temp.hours = 0;
        }
        try {
            validateRecord(temp);
        } catch (DuplicateEntryError& e) {
            if (!ignoreDuplicates) throw e;
        }
        records.emplace_back(temp);
    }
}

HoursData::HoursData( int bookingId, Database* db_ptr) : db(*db_ptr) {
    std::unique_ptr<sql::PreparedStatement> selectStmt;
    selectStmt = db.prepareStatement(
        "SELECT assignmentid, bookeddate, hours FROM Hours WHERE bookingid = ?"
    );
    selectStmt->setInt(1, bookingId);
    auto res = selectStmt->executeQuery();
    Record temp;
    while (res->next()) {
        temp.id = bookingId;
        temp.assignmentId = res->getInt("assignmentid");
        temp.bookedDate = res->getString("bookeddate");
        temp.hours = res->getInt("hours");
        records.emplace_back(temp);
    }
}

void HoursData::setValue(std::string fieldName, std::string fieldValue) {
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
    for (auto& rec : records) {
        if (fieldName == "bookeddate") {
            rec.bookedDate = fieldValue;
        } else if (fieldName == "hours") {
            try {
                rec.hours = std::stoi(fieldValue);
            } catch (const std::exception& e) {
                throw InvalidFieldError("Invalid value for hours. Must be an integer.");
            }
        } else {
            throw InvalidFieldError("Invalid field name. Valid fields are: bookeddate, hours");
        }
        validateRecord(rec);
    }
}

void HoursData::validate() {
    for (auto& record : records) {
        validateRecord(record);
    }
}

void HoursData::validateRecord(Record& record) { 
    if (record.assignmentId <= 0) {
        throw ValidationError("Assignment ID must be a positive integer");
    }
    if (record.hours <= 0) {
        throw ValidationError("Hours must be a positive integer");
    }
    if (record.bookedDate.empty()) {
        throw ValidationError("Booked date cannot be empty");
    }
    if (!utils::isValidDate(record.bookedDate)) {
        throw ValidationError("Booked date must be in YYYY-MM-DD format");
    }
}

} // namespace data
