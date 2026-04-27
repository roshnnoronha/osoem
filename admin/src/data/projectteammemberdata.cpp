#include "projectteammemberdata.h"
#include "../exceptions/exceptions.h"
#include "../utils/utils.h"
#include "../utils/lookup.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace data {

ProjectTeamMemberData::ProjectTeamMemberData(Database& db, int projectId) : db(db) {
    Record temp;
    temp.projectId = projectId;
    temp.employeeId = utils::promptIdOrName(db, "EmployeeId (or Name): ", utils::lookupEmployee,
        [&db](const std::string& prefix) {
            return utils::completionEmployees(db, prefix);
        });
    temp.role = utils::promptInt("Role (0=Member, 1=Lead, 2=Manager): ");
    validateRecord(temp);
    records.emplace_back(temp);
}

ProjectTeamMemberData::ProjectTeamMemberData(Database& db, std::vector<std::map<std::string,std::string>> rows, int projectId, bool ignoreDuplicates): db(db) {
    for (const auto& row : rows) {
        Record temp;
        temp.projectId = projectId;

        if (row.count("employeename") && !row.at("employeename").empty()) {
            temp.employeeId = utils::resolveIdOrName(db, row.at("employeename"), utils::lookupEmployee);
        } else if (row.count("employeeid") && !row.at("employeeid").empty()) {
            temp.employeeId = std::stoi(row.at("employeeid"));
        } else {
            temp.employeeId = 0;
        }

        if (row.count("role") && !row.at("role").empty()) {
            temp.role = std::stoi(row.at("role"));
        } else {
            temp.role = 0;
        }
        try {
            validateRecord(temp);
        } catch (DuplicateEntryError& e) {
            if (!ignoreDuplicates) throw e;
        }
        records.emplace_back(temp);
    }
}

ProjectTeamMemberData::ProjectTeamMemberData( int teamMemberId, Database* db_ptr) : db(*db_ptr) {
    std::unique_ptr<sql::PreparedStatement> selectStmt;
    selectStmt = db.prepareStatement(
        "SELECT projectid, employeeid, role FROM ProjectTeamMembers WHERE teammemberid = ?"
    );
    selectStmt->setInt(1, teamMemberId);
    auto res = selectStmt->executeQuery();
    Record temp;
    while (res->next()) {
        temp.id = teamMemberId;
        temp.projectId = res->getInt("projectid");
        temp.employeeId = res->getInt("employeeid");
        temp.role = res->isNull("role") ? 0 : res->getInt("role");
        records.emplace_back(temp);
    }
}

void ProjectTeamMemberData::setValue(std::string fieldName, std::string fieldValue) {
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
    for (auto& rec : records) {
        if (fieldName == "employeeid" || fieldName == "employeename") {
            // Check if the current employee is associated with any activity or artefact in the project
            auto actStmt = db.prepareStatement(
                "SELECT COUNT(*) AS cnt FROM Activities a "
                "JOIN ActivitySubcategories s ON a.subcategoryid = s.subcategoryid "
                "JOIN ActivityCategories c ON s.categoryid = c.categoryid "
                "WHERE c.projectid = ? AND a.activitymanagerid = ?"
            );
            actStmt->setInt(1, rec.projectId);
            actStmt->setInt(2, rec.employeeId);
            auto actRes = actStmt->executeQuery();
            if (actRes->next() && actRes->getInt("cnt") > 0) {
                throw ValidationError("Cannot change employee: this team member is the activity manager for one or more activities in the project");
            }
            auto artStmt = db.prepareStatement(
                "SELECT COUNT(*) AS cnt FROM Artefacts ar "
                "JOIN ArtefactTypes atype ON ar.artefacttypeid = atype.artefacttypeid "
                "WHERE atype.projectid = ? AND ar.artefactownerid = ?"
            );
            artStmt->setInt(1, rec.projectId);
            artStmt->setInt(2, rec.employeeId);
            auto artRes = artStmt->executeQuery();
            if (artRes->next() && artRes->getInt("cnt") > 0) {
                throw ValidationError("Cannot change employee: this team member is the owner of one or more artefacts in the project");
            }
            rec.employeeId = utils::resolveIdOrName(db, fieldValue, utils::lookupEmployee);
        } else if (fieldName == "role") {
            try {
                rec.role = std::stoi(fieldValue);
            } catch (const std::exception& e) {
                throw InvalidFieldError("Invalid value for role. Must be an integer.");
            }
        } else {
            throw InvalidFieldError("Invalid field name. Valid fields are: employeeid, employeename, role");
        }
        validateRecord(rec);
    }
}

void ProjectTeamMemberData::validate() {
    for (auto& record : records) {
        validateRecord(record);
    }
}

void ProjectTeamMemberData::validateRecord(Record& record) {
    
    if (record.projectId <= 0) {
        throw ValidationError("Project ID must be a positive integer");
    }
    if (record.employeeId <= 0) {
        throw ValidationError("Employee ID must be a positive integer");
    }
    if (record.role < 0 || record.role > 2) {
        throw ValidationError("Role must be 0 (Member), 1 (Lead), or 2 (Manager)");
    }

    // Check for duplicates
    auto stmt = db.prepareStatement(
        "SELECT teammemberid FROM ProjectTeamMembers WHERE projectid = ? AND employeeid = ? AND teammemberid <> ? "
    );
    stmt->setInt(1, record.projectId);
    stmt->setInt(2, record.employeeId);
    stmt->setInt(3, record.id);
    auto res = stmt->executeQuery();
    if (res->next()) {
        throw DuplicateEntryError("team member", "employee " + std::to_string(record.employeeId));
    }
}

} // namespace data
