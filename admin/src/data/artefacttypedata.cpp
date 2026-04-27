#include "artefacttypedata.h"
#include "../exceptions/exceptions.h"
#include "../utils/utils.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace data {

ArtefactTypeData::ArtefactTypeData(Database& db, int projectId) : db(db) {
    Record temp;
    temp.projectId = projectId;
    temp.name = utils::promptString("ArtefactName: ");
    temp.description = utils::promptString("ArtefactDescription: ");
    validateRecord(temp);
    records.emplace_back(temp);
}

ArtefactTypeData::ArtefactTypeData(Database& db, std::vector<std::map<std::string,std::string>> rows, int projectId, bool ignoreDuplicates) : db(db) {
    for (const auto& row : rows) {
        Record temp;
        temp.projectId = projectId;
        temp.name = row.count("artefactname") ? row.at("artefactname") : "";
        temp.description = row.count("artefactdescription") ? row.at("artefactdescription") : "";
        try {
            validateRecord(temp);
        } catch (DuplicateEntryError& e) {
            if (!ignoreDuplicates) throw e;
        }
        records.emplace_back(temp);
    }
}

ArtefactTypeData::ArtefactTypeData( int artefactTypeId, Database* db_ptr) : db(*db_ptr) {
    std::unique_ptr<sql::PreparedStatement> selectStmt;
    selectStmt = db.prepareStatement(
        "SELECT projectid, artefactname, artefactdescription FROM ArtefactTypes WHERE artefacttypeid = ?"
    );
    selectStmt->setInt(1, artefactTypeId);
    auto res = selectStmt->executeQuery();
    Record temp;
    while (res->next()) {
        temp.id = artefactTypeId;
        temp.projectId = res->getInt("projectid");
        temp.name = res->getString("artefactname");
        temp.description = res->getString("artefactdescription");
        records.emplace_back(temp);
    }
}

void ArtefactTypeData::setValue(std::string fieldName, std::string fieldValue) {
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
    for (auto& rec : records) {
        if (fieldName == "artefactname") {
            rec.name = fieldValue;
        } else if (fieldName == "artefactdescription") {
            rec.description = fieldValue;
        } else {
            throw InvalidFieldError("Invalid field name. Valid fields are: artefactname, artefactdescription.");
        }
        validateRecord(rec);
    }
}

void ArtefactTypeData::validate() {
    for (const auto& record : records) {
    }
}

void ArtefactTypeData::validateRecord(Record& record) {

    // Validate project ID is positive
    if (record.projectId <= 0) {
        throw ValidationError("Project ID must be a positive integer");
    }

    // Validate name is not empty
    if (record.name.empty()) {
        throw ValidationError("Artefact type name cannot be empty");
    }
    if (utils::containsReservedChars(record.name)) {
        throw ValidationError("Artefact type name cannot contain reserved characters (,  :  /  ~)");
    }
    if (utils::startsWithNumber(record.name)) {
        throw ValidationError("Artefact type name cannot start with a number");
    }
    if (utils::startsWithDot(record.name)) {
        throw ValidationError("Artefact type name cannot start with '.'");
    }
    if (!utils::isValidLength(record.name, 255)) {
        throw ValidationError("Artefact type name must not exceed 255 characters");
    }

    // Validate description length (when non-empty)
    if (!record.description.empty() && !utils::isValidLength(record.description, 1000)) {
        throw ValidationError("Artefact type description must not exceed 1000 characters");
    }

    // Check for duplicates
    auto stmt = db.prepareStatement(
        "SELECT artefacttypeid FROM ArtefactTypes WHERE projectid = ? AND artefactname = ? AND artefacttypeid <> ?"
    );
    stmt->setInt(1, record.projectId);
    stmt->setString(2, record.name);
    stmt->setInt(3, record.id);
    auto res = stmt->executeQuery();
    if (res->next()) {
        throw DuplicateEntryError("artefact type", record.name);
    }
}

} // namespace data

