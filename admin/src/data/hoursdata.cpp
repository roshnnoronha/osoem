#include "hoursdata.h"
#include "../exceptions/exceptions.h"
#include "../utils/utils.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace data {

namespace {

// Parses an hours value: a positive number with at most two decimal places that fits
// Hours.hours (DECIMAL(5,2), so at most 999.99). Throws InvalidFieldError otherwise.
double parseHours(const std::string& value) {
    const std::string invalid = "Invalid value for hours: '" + value
        + "'. Must be a positive number with at most two decimal places (e.g. 2.5).";
    double hours;
    std::size_t pos = 0;
    try {
        hours = std::stod(value, &pos);
    } catch (const std::exception&) {
        throw InvalidFieldError(invalid);
    }
    std::size_t dot = value.find('.');
    if (pos != value.size()
            || value.find_first_of("eExXnNiI") != std::string::npos
            || (dot != std::string::npos && value.size() - dot - 1 > 2)) {
        throw InvalidFieldError(invalid);
    }
    return hours;
}

} // namespace

HoursData::HoursData(Database& db, int assignmentId) : db(db) {
    Record temp;
    temp.assignmentId = assignmentId;
    temp.bookedDate = utils::promptString("BookedDate (YYYY-MM-DD): ");
    temp.hours = parseHours(utils::promptString("Hours: "));
    validateRecord(temp);
    records.emplace_back(temp);
}

HoursData::HoursData(Database &db, std::vector<std::map<std::string,std::string>> rows, int assignmentId, bool ignoreDuplicates) : db(db) {
    for (const auto& row : rows) {
        Record temp;
        temp.assignmentId = assignmentId;
        temp.bookedDate = row.count("bookeddate") ? row.at("bookeddate") : "";

        if (row.count("hours") && !row.at("hours").empty()) {
            temp.hours = parseHours(row.at("hours"));
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
        temp.hours = res->getDouble("hours");
        records.emplace_back(temp);
    }
}

void HoursData::setValue(std::string fieldName, std::string fieldValue) {
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
    for (auto& rec : records) {
        if (fieldName == "bookeddate") {
            rec.bookedDate = fieldValue;
        } else if (fieldName == "hours") {
            rec.hours = parseHours(fieldValue);
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
        throw ValidationError("Hours must be a positive number");
    }
    if (record.hours > 999.99) {
        throw ValidationError("Hours must not exceed 999.99");
    }
    if (record.bookedDate.empty()) {
        throw ValidationError("Booked date cannot be empty");
    }
    if (!utils::isValidDate(record.bookedDate)) {
        throw ValidationError("Booked date must be in YYYY-MM-DD format");
    }
}

} // namespace data
