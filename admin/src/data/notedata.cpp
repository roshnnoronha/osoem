#include "notedata.h"
#include "../exceptions/exceptions.h"
#include "../utils/utils.h"
#include "../utils/lookup.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace data {

NoteData::NoteData(Database& db, int activityId, int projectId) : db(db) {
    Record temp;
    temp.activityId = activityId;
    temp.note = utils::promptString("Note: ");
    temp.userId = utils::promptIdOrName(db, "UserId (or Name): ", utils::lookupEmployee,
        [&db, projectId](const std::string& prefix) {
            return utils::completionTeamMembers(db, prefix, projectId);
        });
    temp.noteDate = utils::promptString("NoteDate (YYYY-MM-DD): ");
    validateRecord(temp);
    records.emplace_back(temp);
}

NoteData::NoteData(Database& db, std::vector<std::map<std::string,std::string>> rows, int activityId, bool ignoreDuplicates) :db(db) {
    for (const auto& row : rows) {
        Record temp;
        temp.activityId = activityId;

        if (row.count("username") && !row.at("username").empty()) {
            temp.userId = utils::resolveIdOrName(db, row.at("username"), utils::lookupEmployee);
        } else if (row.count("userid") && !row.at("userid").empty()) {
            temp.userId = std::stoi(row.at("userid"));
        } else {
            temp.userId = 0;
        }

        temp.note = row.count("note") ? row.at("note") : "";
        temp.noteDate = row.count("notedate") ? row.at("notedate") : "";
        try {
            validateRecord(temp);
        } catch (DuplicateEntryError& e) {
            if (!ignoreDuplicates) throw e;
        }
        records.emplace_back(temp);
    }
}

NoteData::NoteData( int noteId, Database* db_ptr) : db(*db_ptr) {
    std::unique_ptr<sql::PreparedStatement> selectStmt;
    if (noteId) {
        selectStmt = db.prepareStatement(
            "SELECT activityid, userid, note, notedate FROM ActivityNotes WHERE noteid = ?"
        );
        selectStmt->setInt(1, noteId);
    } else {
        selectStmt = db.prepareStatement(
            "SELECT activityid, userid, note, notedate FROM ActivityNotes"
        );
    }
    auto res = selectStmt->executeQuery();
    Record temp;
    while (res->next()) {
        temp.id = noteId;
        temp.activityId = res->getInt("activityid");
        temp.userId = res->getInt("userid");
        temp.note = res->getString("note");
        temp.noteDate = res->getString("notedate");
        records.emplace_back(temp);
    }
}

void NoteData::setValue(std::string fieldName, std::string fieldValue) {
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
    for (auto& rec : records) {
        if (fieldName == "userid" || fieldName == "username") {
            rec.userId = utils::resolveIdOrName(db, fieldValue, utils::lookupEmployee);
        } else if (fieldName == "note") {
            rec.note = fieldValue;
        } else if (fieldName == "notedate") {
            rec.noteDate = fieldValue;
        } else {
            throw InvalidFieldError("Invalid field name. Valid fields are: userid, username, note, notedate.");
        }
        validateRecord(rec);
    }
}

void NoteData::validate() {
    for (auto& record : records) {
        validateRecord(record);
    }
}

void NoteData::validateRecord(Record& record) {
    // Validate activity ID is positive
    if (record.activityId <= 0) {
        throw ValidationError("Activity ID must be a positive integer");
    }

    // Validate user ID is positive
    if (record.userId <= 0) {
        throw ValidationError("User ID must be a positive integer");
    }

    // Validate note is not empty
    if (record.note.empty()) {
        throw ValidationError("Note cannot be empty");
    }
    if (!utils::isValidLength(record.note, 2000)) {
        throw ValidationError("Note must not exceed 2000 characters");
    }

    // Validate note date is not empty
    if (record.noteDate.empty()) {
        throw ValidationError("Note date cannot be empty");
    }
    if (!utils::isValidDate(record.noteDate)) {
        throw ValidationError("Note date must be in YYYY-MM-DD format");
    }

    // Check for duplicates
    auto stmt = db.prepareStatement(
        "SELECT noteid FROM ActivityNotes WHERE activityid = ? AND userid = ? AND note = ? AND notedate = ? AND noteid <> ?"
    );
    stmt->setInt(1, record.activityId);
    stmt->setInt(2, record.userId);
    stmt->setString(3, record.note);
    stmt->setString(4, record.noteDate);
    stmt->setInt(5, record.id);
    auto res = stmt->executeQuery();
    if (res->next()) {
        throw DuplicateEntryError("note", record.noteDate + " by user " + std::to_string(record.userId));
    }
}
} // namespace data

