#include "milestonetoartefactlinkdata.h"
#include "../exceptions/exceptions.h"
#include "../utils/utils.h"
#include "../utils/lookup.h"
#include "../utils/readline_wrapper.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace data {

MilestoneToArtefactLinkData::MilestoneToArtefactLinkData(Database& db ,int artefactLinkId, int projectId) : db(db), projectId(projectId) {
    Record temp;
    temp.artefactLinkId = artefactLinkId;
    temp.milestoneStepId = utils::promptIdOrNameScoped(db, "MilestoneStepId (or Name): ", projectId, utils::lookupMilestoneStepByProject,
        [&db, projectId](const std::string& prefix) {
            return utils::completionMilestoneStepsByProject(db, prefix, projectId);
        });
    std::string input = readline_wrapper::readLineWithCompletion("CompletedById (or Name, 0 if not complete): ",
        [&db, projectId](const std::string& prefix) {
            return utils::completionTeamMembers(db, prefix, projectId);
        });
    if (input == "0" || input.empty()) {
        temp.completedById = 0;
    } else {
        // Try to parse as integer first
        try {
            temp.completedById = std::stoi(input);
        } catch (const std::exception&) {
            utils::LookupResult result = utils::lookupEmployee(db, input);
            if (result.multipleMatches) {
                throw ValidationError("Multiple employees found with name \"" + input + "\". Please use ID instead.");
            }
            if (!result.found) {
                throw ValidationError("No employee found with name \"" + input + "\"");
            }
            temp.completedById = result.id;
        }
    }
    temp.completionDate = utils::promptString("CompletionDate (YYYY-MM-DD, optional): ");
    validateRecord(temp);
    records.emplace_back(temp);
}

MilestoneToArtefactLinkData::MilestoneToArtefactLinkData(Database& db, std::vector<std::map<std::string,std::string>> rows, int artefactLinkId, int projectId, bool ignoreDuplicates) : db(db), projectId(projectId) {
    for (const auto& row : rows) {
        Record temp;
        temp.artefactLinkId = artefactLinkId;

        if (row.count("milestonestepname") && !row.at("milestonestepname").empty() && projectId > 0) {
            temp.milestoneStepId = utils::resolveIdOrNameScoped(db, row.at("milestonestepname"), projectId, utils::lookupMilestoneStepByProject);
        } else if (row.count("milestonestepid") && !row.at("milestonestepid").empty()) {
            temp.milestoneStepId = std::stoi(row.at("milestonestepid"));
        } else {
            temp.milestoneStepId = 0;
        }

        if (row.count("completedbyname") && !row.at("completedbyname").empty()) {
            temp.completedById = utils::resolveIdOrName(db, row.at("completedbyname"), utils::lookupEmployee);
        } else if (row.count("completedbyid") && !row.at("completedbyid").empty()) {
            temp.completedById = std::stoi(row.at("completedbyid"));
        } else {
            temp.completedById = 0;
        }

        temp.completionDate = row.count("completiondate") ? row.at("completiondate") : "";
        try {
            validateRecord(temp);
        } catch (DuplicateEntryError& e) {
            if (!ignoreDuplicates) throw e;
        }
        records.emplace_back(temp);
    }
}

MilestoneToArtefactLinkData::MilestoneToArtefactLinkData( int milestoneLinkId, Database* db_ptr) : db(*db_ptr) {
    std::unique_ptr<sql::PreparedStatement> selectStmt;
    selectStmt = db.prepareStatement(
        "SELECT ml.milestonestepid, ml.artefactlinkid, ml.completedbyid, ml.completiondate, m.projectid "
        "FROM MilestoneToArtefactLink ml "
        "JOIN MilestoneSteps ms ON ms.milestonestepid = ml.milestonestepid "
        "JOIN Milestones m ON m.milestoneid = ms.milestoneid "
        "WHERE ml.milestonelinkid = ?"
    );
    selectStmt->setInt(1, milestoneLinkId);
    auto res = selectStmt->executeQuery();
    Record temp;
    while (res->next()) {
        temp.id = milestoneLinkId;
        temp.milestoneStepId = res->getInt("milestonestepid");
        temp.artefactLinkId = res->getInt("artefactlinkid");
        temp.completedById = res->isNull("completedbyid") ? 0 : res->getInt("completedbyid");
        temp.completionDate = res->isNull("completiondate") ? "" : res->getString("completiondate");
        projectId = res->getInt("projectid");
        records.emplace_back(temp);
    }
}

void MilestoneToArtefactLinkData::setValue(std::string fieldName, std::string fieldValue) {
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
    for (auto& rec : records) {
        if (fieldName == "milestonestepid" || fieldName == "milestonestepname") {
            rec.milestoneStepId = utils::resolveIdOrNameScoped(db, fieldValue, projectId, utils::lookupMilestoneStepByProject);
        } else if (fieldName == "completedbyid" || fieldName == "completedby" || fieldName == "completedbyname") {
            if (fieldValue.empty()) {
                rec.completedById = 0;
            } else {
                rec.completedById = utils::resolveIdOrName(db, fieldValue, utils::lookupEmployee);
            }
        } else if (fieldName == "completiondate") {
            rec.completionDate = fieldValue;
        } else {
            throw InvalidFieldError("Invalid field name. Valid fields are: milestonestepid, milestonestepname, completedbyid, completedbyname, completiondate");
        }
        validateRecord(rec);
    }
}

void MilestoneToArtefactLinkData::validate() {
    for (auto& record : records) {
        validateRecord(record);
    }
}

void MilestoneToArtefactLinkData::validateRecord(Record& record) {
    if (record.artefactLinkId <= 0) {
        throw ValidationError("Artefact link ID must be a positive integer");
    }
    if (record.milestoneStepId <= 0) {
        throw ValidationError("Milestone step ID must be a positive integer");
    }
    // Validate completion date format (when non-empty)
    if (!record.completionDate.empty() && !utils::isValidDate(record.completionDate)) {
        throw ValidationError("Completion date must be in YYYY-MM-DD format");
    }
    // Validate completedById is non-negative (0 means not completed)
    if (record.completedById < 0) {
        throw ValidationError("Completed by ID must be non-negative");
    }

    // Check for duplicates
    auto stmt = db.prepareStatement(
        "SELECT milestonelinkid FROM MilestoneToArtefactLink WHERE artefactlinkid = ? AND milestonestepid = ? AND milestonelinkid <> ?"
    );
    stmt->setInt(1, record.artefactLinkId);
    stmt->setInt(2, record.milestoneStepId);
    stmt->setInt(3, record.id);
    auto res = stmt->executeQuery();
    if (res->next()) {
        throw DuplicateEntryError("milestone-artefact link", "milestone step " + std::to_string(record.milestoneStepId));
    }

}
} // namespace data
