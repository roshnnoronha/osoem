#include "artefactdata.h"
#include "../exceptions/exceptions.h"
#include "../utils/utils.h"
#include "../utils/lookup.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace data {

ArtefactData::ArtefactData(Database& db, int artefactTypeId, int projectId) : db(db){
    Record temp;
    temp.artefactTypeId = artefactTypeId;
    temp.title = utils::promptString("ArtefactTitle: ");
    temp.artefactOwnerId = utils::promptIdOrName(db, "ArtefactOwnerId (or Name): ", utils::lookupEmployee,
        [&db, projectId](const std::string& prefix) {
            return utils::completionTeamMembers(db, prefix, projectId);
        });
    validateRecord(temp);
    records.emplace_back(temp);
}

ArtefactData::ArtefactData(Database& db, std::vector<std::map<std::string,std::string>> rows, int artefactTypeId, bool ignoreDuplicates) : db(db) {
    for (const auto& row : rows) {
        Record temp;
        temp.artefactTypeId = artefactTypeId;
        temp.title = row.count("artefacttitle") ? row.at("artefacttitle") : "";
        if (row.count("artefactownername") && !row.at("artefactownername").empty()) {
            temp.artefactOwnerId = utils::resolveIdOrName(db, row.at("artefactownername"), utils::lookupEmployee);
        } else if (row.count("artefactownerid") && !row.at("artefactownerid").empty()) {
            temp.artefactOwnerId = std::stoi(row.at("artefactownerid"));
        } else {
            temp.artefactOwnerId = 0;
        }
        try {
            validateRecord(temp);
        } catch (DuplicateEntryError& e) {
            if (!ignoreDuplicates) throw e;
        }
        records.emplace_back(temp);
    }
}

ArtefactData::ArtefactData(int artefactId, Database* db_ptr) : db(*db_ptr) {
    std::unique_ptr<sql::PreparedStatement> selectStmt;
    selectStmt = db.prepareStatement(
        "SELECT artefacttypeid, artefacttitle, artefactownerid FROM Artefacts WHERE artefactid = ?"
    );
    selectStmt->setInt(1, artefactId);
    auto res = selectStmt->executeQuery();
    Record temp;
    while (res->next()) {
        temp.id = artefactId;
        temp.artefactTypeId = res->getInt("artefacttypeid");
        temp.title = res->getString("artefacttitle");
        temp.artefactOwnerId = res->getInt("artefactownerid");
        records.emplace_back(temp);
        validateRecord(temp);
    }
}

void ArtefactData::setValue(std::string fieldName, std::string fieldValue) {
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
    for (auto& rec : records) {
        if (fieldName == "artefacttypeid") {
            try {
                rec.artefactTypeId = std::stoi(fieldValue);
            } catch (const std::exception& e) {
                throw InvalidFieldError("Invalid value for artefacttypeid. Must be an integer.");
            }
        } else if (fieldName == "artefacttitle") {
            rec.title = fieldValue;
        } else if (fieldName == "artefactownerid" || fieldName == "artefactownername") {
            int newOwnerId = utils::resolveIdOrName(db, fieldValue, utils::lookupEmployee);
            auto teamStmt = db.prepareStatement(
                "SELECT COUNT(*) AS cnt FROM ProjectTeamMembers ptm "
                "JOIN ArtefactTypes atype ON ptm.projectid = atype.projectid "
                "WHERE atype.artefacttypeid = ? AND ptm.employeeid = ?"
            );
            teamStmt->setInt(1, rec.artefactTypeId);
            teamStmt->setInt(2, newOwnerId);
            auto teamRes = teamStmt->executeQuery();
            if (teamRes->next() && teamRes->getInt("cnt") == 0) {
                throw ValidationError("Artefact owner must be a member of the project team");
            }
            rec.artefactOwnerId = newOwnerId;
        } else {
            throw InvalidFieldError("Invalid field name:" + fieldName + ". Valid fields are: artefacttypeid, artefacttitle, artefactownerid, artefactownername.");
        }
    }
}

void ArtefactData::validate() {
    for (auto& record : records) {
        validateRecord(record);
    }
}

void ArtefactData::validateRecord(Record& record) { 
    // Validate artefact type ID is positive
    if (record.artefactTypeId <= 0) {
        throw ValidationError("Artefact type ID must be a positive integer");
    }

    // Validate title is not empty
    if (record.title.empty()) {
        throw ValidationError("Artefact title cannot be empty");
    }
    if (utils::containsReservedChars(record.title)) {
        throw ValidationError("Artefact title cannot contain reserved characters (,  :  /  ~)");
    }
    if (utils::startsWithNumber(record.title)) {
        throw ValidationError("Artefact title cannot start with a number");
    }
    if (utils::startsWithDot(record.title)) {
        throw ValidationError("Artefact title cannot start with '.'");
    }
    if (!utils::isValidLength(record.title, 255)) {
        throw ValidationError("Artefact title must not exceed 255 characters");
    }

    // Validate artefact owner ID is positive
    if (record.artefactOwnerId <= 0) {
        throw ValidationError("Artefact owner ID must be a positive integer");
    }

    // Validate artefact owner is a member of the project team (only for new records)
    if (record.id == 0) {
        auto teamStmt = db.prepareStatement(
            "SELECT COUNT(*) AS cnt FROM ProjectTeamMembers ptm "
            "JOIN ArtefactTypes atype ON ptm.projectid = atype.projectid "
            "WHERE atype.artefacttypeid = ? AND ptm.employeeid = ?"
        );
        teamStmt->setInt(1, record.artefactTypeId);
        teamStmt->setInt(2, record.artefactOwnerId);
        auto teamRes = teamStmt->executeQuery();
        if (teamRes->next() && teamRes->getInt("cnt") == 0) {
            throw ValidationError("Artefact owner must be a member of the project team");
        }
    }

    // Check for duplicates
    auto stmt = db.prepareStatement(
        "SELECT artefactid FROM Artefacts WHERE artefacttypeid = ? AND artefacttitle = ? AND artefactid <> ?"
    );
    stmt->setInt(1, record.artefactTypeId);
    stmt->setString(2, record.title);
    stmt->setInt(3,record.id);
    auto res = stmt->executeQuery();
    if (res->next()) {
        throw DuplicateEntryError("artefact", record.title);
    }

}

} // namespace data

