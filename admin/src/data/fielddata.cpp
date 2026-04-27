#include "fielddata.h"
#include "../exceptions/exceptions.h"
#include "../utils/utils.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace data {

FieldData::FieldData(Database& db, int artefactTypeId) : db(db) {
    Record temp;
    temp.artefactTypeId = artefactTypeId;
    temp.title = utils::promptString("FieldTitle: ");
    temp.valueType = utils::promptInt("ValueType (0=text, 1=numeric): ");
    switch (temp.valueType) {
    case 0:
        temp.maximumLength = utils::promptInt("MaximumLength: ");
        temp.maximumValue = 0;
        temp.minimumValue = 0;
        break;
    case 1:
        temp.maximumLength = 0;
        temp.maximumValue = utils::promptInt("MaximumValue: ");
        temp.minimumValue = utils::promptInt("MinimumValue: ");
        break;
    default:
        std::cerr << "Error: Invalid value type.";
    }
    validateRecord(temp);
    records.emplace_back(temp);
}

FieldData::FieldData(Database& db, std::vector<std::map<std::string,std::string>> rows, int artefactTypeId, bool ignoreDuplicates) : db(db) {
    for (const auto& row : rows) {
        Record temp;
        temp.artefactTypeId = artefactTypeId;
        temp.title = row.count("fieldtitle") ? row.at("fieldtitle") : "";

        if (row.count("valuetype") && !row.at("valuetype").empty()) {
            if (utils::isNumber(row.at("valuetype")))
                temp.valueType = std::stoi(row.at("valuetype"));
            else
                throw ValidationError("ValueType should be a number (0=text, 1=numeric).");
        } else {
            temp.valueType = 0;
        }

        if (row.count("maximumlength") && !row.at("maximumlength").empty()) {
            temp.maximumLength = std::stoi(row.at("maximumlength"));
        } else {
            temp.maximumLength = 0;
        }

        if (row.count("maximumvalue") && !row.at("maximumvalue").empty()) {
            temp.maximumValue = std::stoi(row.at("maximumvalue"));
        } else {
            temp.maximumValue = 0;
        }

        if (row.count("minimumvalue") && !row.at("minimumvalue").empty()) {
            temp.minimumValue = std::stoi(row.at("minimumvalue"));
        } else {
            temp.minimumValue = 0;
        }
        try {
            validateRecord(temp);
        } catch (DuplicateEntryError& e) {
            if (!ignoreDuplicates) throw e;
        }
        records.emplace_back(temp);
    }
}

FieldData::FieldData( int fieldId, Database* db_ptr) : db(*db_ptr) {
    std::unique_ptr<sql::PreparedStatement> selectStmt;
    if (fieldId) {
        selectStmt = db.prepareStatement(
            "SELECT artefacttypeid, fieldtitle, valuetype, maximumlength, maximumvalue, minimumvalue FROM ArtefactDataFields WHERE artefactdatafieldid = ?"
        );
        selectStmt->setInt(1, fieldId);
    } else {
        selectStmt = db.prepareStatement(
            "SELECT artefacttypeid, fieldtitle, valuetype, maximumlength, maximumvalue, minimumvalue FROM ArtefactDataFields"
        );
    }
    auto res = selectStmt->executeQuery();
    Record temp;
    while (res->next()) {
        temp.id = fieldId;
        temp.artefactTypeId = res->getInt("artefacttypeid");
        temp.title = res->getString("fieldtitle");
        temp.valueType = res->getInt("valuetype");
        temp.maximumLength = res->getInt("maximumlength");
        temp.maximumValue = res->getInt("maximumvalue");
        temp.minimumValue = res->getInt("minimumvalue");
        records.emplace_back(temp);
    }
}

void FieldData::setValue(std::string fieldName, std::string fieldValue) {
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
    for (auto& rec : records) {
        if (fieldName == "fieldtitle") {
            rec.title = fieldValue;
        } else if (fieldName == "valuetype") {
            try {
                rec.valueType = std::stoi(fieldValue);
            } catch (const std::exception& e) {
                throw InvalidFieldError("Invalid value for valuetype. Must be an integer.");
            }
        } else if (fieldName == "maximumlength") {
            try {
                rec.maximumLength = std::stoi(fieldValue);
            } catch (const std::exception& e) {
                throw InvalidFieldError("Invalid value for maximumlength. Must be an integer.");
            }
        } else if (fieldName == "maximumvalue") {
            try {
                rec.maximumValue = std::stoi(fieldValue);
            } catch (const std::exception& e) {
                throw InvalidFieldError("Invalid value for maximumvalue. Must be an integer.");
            }
        } else if (fieldName == "minimumvalue") {
            try {
                rec.minimumValue = std::stoi(fieldValue);
            } catch (const std::exception& e) {
                throw InvalidFieldError("Invalid value for minimumvalue. Must be an integer.");
            }
        } else {
            throw InvalidFieldError("Invalid field name. Valid fields are: fieldtitle, valuetype, maximumlength, maximumvalue, minimumvalue.");
        }
        validateRecord(rec);
    }
}

void FieldData::validate() {
    for (auto& record : records) {
        validateRecord(record);
    }
}

void FieldData::validateRecord(Record& record) {
    // Validate artefact type ID is positive
    if (record.artefactTypeId <= 0) {
        throw ValidationError("Artefact type ID must be a positive integer");
    }

    // Validate title is not empty
    if (record.title.empty()) {
        throw ValidationError("Field title cannot be empty");
    }
    if (utils::containsReservedChars(record.title)) {
        throw ValidationError("Field title cannot contain reserved characters (,  :  /  ~)");
    }
    if (utils::startsWithNumber(record.title)) {
        throw ValidationError("Field title cannot start with a number");
    }
    if (utils::startsWithDot(record.title)) {
        throw ValidationError("Field title cannot start with '.'");
    }
    if (!utils::isValidLength(record.title, 255)) {
        throw ValidationError("Field title must not exceed 255 characters");
    }

    // Validate value type is 0 or 1
    if (record.valueType != 0 && record.valueType != 1) {
        throw ValidationError("Value type must be 0 (text) or 1 (numeric)");
    }

    // Validate based on value type
    if (record.valueType == 0) {
        if (record.maximumLength <= 0) {
            throw ValidationError("Maximum length must be positive for text fields");
        }
        if (record.maximumLength > 2000) {
            throw ValidationError("Maximum length must not exceed 2000 for text fields");
        }
    } else if (record.valueType == 1) {
        if (record.minimumValue > record.maximumValue) {
            throw ValidationError("Minimum value cannot exceed maximum value");
        }
    }

    // Check for duplicates
    auto stmt = db.prepareStatement(
        "SELECT artefactdatafieldid FROM ArtefactDataFields WHERE artefacttypeid = ? AND fieldtitle = ? AND artefactdatafieldid <> ?"
    );
    stmt->setInt(1, record.artefactTypeId);
    stmt->setString(2, record.title);
    stmt->setInt(3, record.id);
    auto res = stmt->executeQuery();
    if (res->next()) {
        throw DuplicateEntryError("field", record.title);
    }
}
} // namespace data

