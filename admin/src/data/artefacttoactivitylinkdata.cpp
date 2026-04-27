#include "artefacttoactivitylinkdata.h"
#include "../exceptions/exceptions.h"
#include "../utils/utils.h"
#include "../utils/lookup.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace data {

ArtefactToActivityLinkData::ArtefactToActivityLinkData(Database& db, int artefactId, int projectId) : db(db) {
    Record temp;
    temp.artefactId = artefactId;
    temp.activityId = utils::promptIdOrNameScoped(db, "ActivityId (or Name): ", projectId, utils::lookupActivityByProject,
        [&db, projectId](const std::string& prefix) {
            return utils::completionActivitiesByProject(db, prefix, projectId);
        });
    temp.ratio = utils::promptDouble("Ratio (0.0-1.0): ");
    validateRecord(temp);
    records.emplace_back(temp);
}

ArtefactToActivityLinkData::ArtefactToActivityLinkData(Database& db, std::vector<std::map<std::string,std::string>> rows, int artefactId, int projectId, bool ignoreDuplicates) : db(db) {
    for (const auto& row : rows) {
        Record temp;
        temp.artefactId = artefactId;

        if (row.count("activityname") && !row.at("activityname").empty()) {
            const std::string& actName = row.at("activityname");
            // Look up by name, optionally scoped by category and/or subcategory
            std::string catName    = (row.count("categoryname")    && !row.at("categoryname").empty())    ? row.at("categoryname")    : "";
            std::string subCatName = (row.count("subcategoryname") && !row.at("subcategoryname").empty()) ? row.at("subcategoryname") : "";
            utils::LookupResult lr = utils::lookupActivityByNames(db, actName, projectId, catName, subCatName);
            if (lr.multipleMatches) {
                throw ValidationError("Multiple activities found with name \"" + actName + "\". "
                    "Provide 'categoryname' and/or 'subcategoryname' columns to disambiguate.");
            }
            if (!lr.found) {
                throw ValidationError("No activity found with name \"" + (!catName.empty() ? catName + "/" : "") + (!subCatName.empty() ? subCatName + "/" : "")  + actName + "\".");
            }
            temp.activityId = lr.id;
        } else if (row.count("activityid") && !row.at("activityid").empty()) {
            temp.activityId = std::stoi(row.at("activityid"));
        } else {
            temp.activityId = 0;
        }

        if (row.count("ratio") && !row.at("ratio").empty()) {
            temp.ratio = std::stod(row.at("ratio"));
        } else {
            temp.ratio = 0.0;
        }
        try {
            validateRecord(temp);
        } catch (DuplicateEntryError& e) {
            if (!ignoreDuplicates) throw e;
        }
        records.emplace_back(temp);
    }
}

ArtefactToActivityLinkData::ArtefactToActivityLinkData( int artefactLinkId,Database* db_ptr) : db(*db_ptr) {
    std::unique_ptr<sql::PreparedStatement> selectStmt;
    selectStmt = db.prepareStatement(
        "SELECT activityid, artefactid, ratio FROM ArtefactToActivityLink WHERE artefactlinkid = ?"
    );
    selectStmt->setInt(1, artefactLinkId);
    auto res = selectStmt->executeQuery();
    Record temp;
    while (res->next()) {
        temp.id = artefactLinkId;
        temp.activityId = res->getInt("activityid");
        temp.artefactId = res->getInt("artefactid");
        temp.ratio = res->isNull("ratio") ? 0.0 : res->getDouble("ratio");
        records.emplace_back(temp);
    }
}

void ArtefactToActivityLinkData::setValue(std::string fieldName, std::string fieldValue) {
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
    for (auto& rec : records) {
        if (fieldName == "activityid" || fieldName == "activityname") {
            rec.activityId = utils::resolveIdOrName(db, fieldValue, utils::lookupActivity);
        } else if (fieldName == "ratio") {
            try {
                rec.ratio = std::stod(fieldValue);
            } catch (const std::exception& e) {
                throw InvalidFieldError("Invalid value for ratio. Must be a decimal number.");
            }
        } else {
            throw InvalidFieldError("Invalid field name. Valid fields are: activityid, activityname, ratio");
        }
        validateRecord(rec);
    }
}

void ArtefactToActivityLinkData::validate() {
    for (auto& record : records) {
        validateRecord(record);
    }
}

void ArtefactToActivityLinkData::validateRecord(Record& record) {

    if (record.artefactId <= 0) {
        throw ValidationError("Artefact ID must be a positive integer");
    }
    //TODO: Check id artefact Id is valid
    if (record.activityId <= 0) {
        throw ValidationError("Activity ID must be a positive integer");
    }
    //TODO: Check if activity Id is valid

    if (record.ratio < 0.0 || record.ratio > 1.0) {
        throw ValidationError("Ratio must be between 0.0 and 1.0");
    }

    // Check for duplicates
    auto stmt = db.prepareStatement(
        "SELECT artefactlinkid FROM ArtefactToActivityLink WHERE artefactid = ? AND activityid = ? AND artefactlinkid <> ?"
    );
    stmt->setInt(1, record.artefactId);
    stmt->setInt(2, record.activityId);
    stmt->setInt(3, record.id);
    auto res = stmt->executeQuery();
    if (res->next()) {
        throw DuplicateEntryError("artefact-activity link",
            "artefact " + std::to_string(record.artefactId) + " to activity " + std::to_string(record.activityId));
    }
}

} // namespace data
