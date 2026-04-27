#include "artefactdataitemdata.h"
#include "../exceptions/exceptions.h"
#include "../utils/utils.h"
#include "../utils/lookup.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace data {

ArtefactDataItemData::ArtefactDataItemData(Database& db, int artefactId, int artefactTypeId) : db(db) {
    Record temp;
    temp.artefactId = artefactId;
    // Display available fields before prompting
    auto fieldStmt = db.prepareStatement(
       "SELECT artefactdatafieldid, fieldtitle FROM ArtefactDataFields WHERE artefacttypeid = ? ORDER BY fieldtitle"
    );
    fieldStmt->setInt(1, artefactTypeId);
    auto fieldRes = fieldStmt->executeQuery();

    bool hasFields = false;
    std::cout << "Available fields:" << std::endl;
    while (fieldRes->next()) {
        hasFields = true;
       std::cout << "  (" << fieldRes->getInt("artefactdatafieldid") << ") "
                  << fieldRes->getString("fieldtitle") << std::endl;
    }
    if (!hasFields) {
        // Get artefact type name for a better error message
        auto typeStmt = db.prepareStatement(
            "SELECT artefactname FROM ArtefactTypes WHERE artefacttypeid = ?"
        );
        typeStmt->setInt(1, artefactTypeId);
        auto typeRes = typeStmt->executeQuery();
        std::string typeName;
        if (typeRes->next()) {
            typeName = typeRes->getString("artefactname");
        } else {
            typeName = std::to_string(artefactTypeId);
        }
        throw ValidationError("No data fields are defined for artefact type '" + typeName + "'");
    }
    temp.artefactDataFieldId = utils::promptIdOrNameScoped(db, "ArtefactDataFieldId (or Name): ", artefactTypeId, utils::lookupArtefactDataField);
    temp.value = utils::promptString("Value: ");
    validateRecord(temp);
    records.emplace_back(temp);
}

ArtefactDataItemData::ArtefactDataItemData(Database& db, std::vector<std::map<std::string,std::string>> rows, int artefactId, int artefactTypeId, bool ignoreDuplicates) : db(db) {
    for (const auto& row : rows) {
        Record temp;
        temp.artefactId = artefactId;
        if (row.count("fieldname") && !row.at("fieldname").empty() && artefactTypeId > 0) {
            temp.artefactDataFieldId = utils::resolveIdOrNameScoped(db, row.at("fieldname"), artefactTypeId, utils::lookupArtefactDataField);
        } else if (row.count("artefactdatafieldid") && !row.at("artefactdatafieldid").empty()) {
            temp.artefactDataFieldId = std::stoi(row.at("artefactdatafieldid"));
        } else {
            temp.artefactDataFieldId = 0;
        }
        temp.value = row.count("value") ? row.at("value") : "";
        try {
            validateRecord(temp);
        } catch (DuplicateEntryError& e) {
            if (!ignoreDuplicates) throw e;
        }
        records.emplace_back(temp);
    }
}

ArtefactDataItemData::ArtefactDataItemData(int artefactDataId, Database* db_ptr) : db(*db_ptr) {
    std::unique_ptr<sql::PreparedStatement> selectStmt;
    selectStmt = db.prepareStatement(
        "SELECT artefactid, artefactdatafieldid, value FROM ArtefactData WHERE artefactdataid = ?"
    );
    selectStmt->setInt(1, artefactDataId);
    auto res = selectStmt->executeQuery();
    Record temp;
    while (res->next()) {
        temp.id = artefactDataId;
        temp.artefactId = res->getInt("artefactid");
        temp.artefactDataFieldId = res->getInt("artefactdatafieldid");
        temp.value = res->isNull("value") ? "" : res->getString("value");
        records.emplace_back(temp);
    }
}

void ArtefactDataItemData::setValue(std::string fieldName, std::string fieldValue) {
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
    for (auto& rec : records) {
        if (fieldName == "value") {
            rec.value = fieldValue;
        } else {
            throw InvalidFieldError("Invalid field name. Valid fields are: value");
        }
        validateRecord(rec);
    }
}

void ArtefactDataItemData::validate() {
    for (auto& record : records) {
        validateRecord(record);
    }
}

void ArtefactDataItemData::validateRecord(Record& record) {
    if (record.artefactId <= 0) {
        throw ValidationError("Artefact ID must be a positive integer");
    }
    if (record.artefactDataFieldId <= 0) {
        throw ValidationError("Artefact data field ID must be a positive integer");
    }
    // Validate value length (when non-empty)
    if (!record.value.empty() && !utils::isValidLength(record.value, 2000)) {
        throw ValidationError("Value must not exceed 2000 characters");
    }

    // Check for duplicates
    auto stmt = db.prepareStatement(
        "SELECT artefactdataid FROM ArtefactData WHERE artefactid = ? AND artefactdatafieldid = ? AND artefactdataid <> ?"
    );
    stmt->setInt(1, record.artefactId);
    stmt->setInt(2, record.artefactDataFieldId);
    stmt->setInt(3, record.id);
    auto res = stmt->executeQuery();
    if (res->next()) {
        throw DuplicateEntryError("artefact data", "field " + std::to_string(record.artefactDataFieldId));
    }

}

} // namespace data
