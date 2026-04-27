#include "lookup.h"
#include "utils.h"
#include "readline_wrapper.h"
#include "../exceptions/exceptions.h"
#include <iostream>

namespace utils {

LookupResult lookupEmployee(data::Database& db, const std::string& name) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT employeeid FROM Employees WHERE CONCAT(firstname, ' ', lastname) = ?"
    );
    stmt->setString(1, name);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("employeeid");
        result.found = true;
        // Check for multiple matches
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupDepartment(data::Database& db, const std::string& name) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT departmentid FROM Departments WHERE departmentname = ?"
    );
    stmt->setString(1, name);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("departmentid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupActivity(data::Database& db, const std::string& name) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT activityid FROM Activities WHERE activityname = ?"
    );
    stmt->setString(1, name);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("activityid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupMilestoneStep(data::Database& db, const std::string& name, int milestoneId) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT milestonestepid FROM MilestoneSteps WHERE milestonestepname = ? AND milestoneid = ?"
    );
    stmt->setString(1, name);
    stmt->setInt(2, milestoneId);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("milestonestepid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupActivityByProject(data::Database& db, const std::string& name, int projectId) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT a.activityid FROM Activities a "
        "JOIN ActivitySubcategories s ON a.subcategoryid = s.subcategoryid "
        "JOIN ActivityCategories c ON s.categoryid = c.categoryid "
        "WHERE a.activityname = ? AND c.projectid = ?"
    );
    stmt->setString(1, name);
    stmt->setInt(2, projectId);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("activityid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupActivityByNames(data::Database& db, const std::string& activityName, int projectId, const std::string& categoryName, const std::string& subcategoryName) {
    LookupResult result = {0, false, false};

    std::string query =
        "SELECT a.activityid FROM Activities a "
        "JOIN ActivitySubcategories s ON a.subcategoryid = s.subcategoryid "
        "JOIN ActivityCategories c ON s.categoryid = c.categoryid "
        "WHERE a.activityname = ? AND c.projectid = ?";
    if (!categoryName.empty())    query += " AND c.categoryname = ?";
    if (!subcategoryName.empty()) query += " AND s.subcategoryname = ?";

    auto stmt = db.prepareStatement(query);
    int p = 1;
    stmt->setString(p++, activityName);
    stmt->setInt(p++, projectId);
    if (!categoryName.empty())    stmt->setString(p++, categoryName);
    if (!subcategoryName.empty()) stmt->setString(p++, subcategoryName);

    auto res = stmt->executeQuery();
    if (res->next()) {
        result.id = res->getInt("activityid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupMilestoneStepByProject(data::Database& db, const std::string& name, int projectId) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT ms.milestonestepid FROM MilestoneSteps ms "
        "JOIN Milestones m ON ms.milestoneid = m.milestoneid "
        "WHERE ms.milestonestepname = ? AND m.projectid = ?"
    );
    stmt->setString(1, name);
    stmt->setInt(2, projectId);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("milestonestepid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupProject(data::Database& db, const std::string& name) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT projectid FROM Projects WHERE projectname = ?"
    );
    stmt->setString(1, name);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("projectid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupCategory(data::Database& db, const std::string& name, int projectId) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT categoryid FROM ActivityCategories WHERE categoryname = ? AND projectid = ?"
    );
    stmt->setString(1, name);
    stmt->setInt(2, projectId);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("categoryid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupSubcategory(data::Database& db, const std::string& name, int categoryId) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT subcategoryid FROM ActivitySubcategories WHERE subcategoryname = ? AND categoryid = ?"
    );
    stmt->setString(1, name);
    stmt->setInt(2, categoryId);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("subcategoryid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupActivityBySubcategory(data::Database& db, const std::string& name, int subcategoryId) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT activityid FROM Activities WHERE activityname = ? AND subcategoryid = ?"
    );
    stmt->setString(1, name);
    stmt->setInt(2, subcategoryId);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("activityid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupTaskByParent(data::Database& db, const std::string& name, int parentTaskId) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT taskid FROM ActivityTasks WHERE taskname = ? AND parenttaskid = ?"
    );
    stmt->setString(1, name);
    stmt->setInt(2, parentTaskId);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("taskid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupTaskByActivity(data::Database& db, const std::string& name, int activityId) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT taskid FROM ActivityTasks WHERE taskname = ? AND activityid = ? AND parenttaskid IS NULL"
    );
    stmt->setString(1, name);
    stmt->setInt(2, activityId);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("taskid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupTaskByNames(data::Database& db, const std::string& taskName, int projectId,
                               const std::string& activityName,
                               const std::string& subcategoryName,
                               const std::string& categoryName) {
    LookupResult result = {0, false, false};

    std::string query =
        "SELECT t.taskid FROM ActivityTasks t "
        "JOIN Activities a ON t.activityid = a.activityid "
        "JOIN ActivitySubcategories s ON a.subcategoryid = s.subcategoryid "
        "JOIN ActivityCategories c ON s.categoryid = c.categoryid "
        "WHERE t.taskname = ? AND c.projectid = ?";
    if (!activityName.empty())    query += " AND a.activityname = ?";
    if (!subcategoryName.empty()) query += " AND s.subcategoryname = ?";
    if (!categoryName.empty())    query += " AND c.categoryname = ?";

    auto stmt = db.prepareStatement(query);
    int p = 1;
    stmt->setString(p++, taskName);
    stmt->setInt(p++, projectId);
    if (!activityName.empty())    stmt->setString(p++, activityName);
    if (!subcategoryName.empty()) stmt->setString(p++, subcategoryName);
    if (!categoryName.empty())    stmt->setString(p++, categoryName);

    auto res = stmt->executeQuery();
    if (res->next()) {
        result.id = res->getInt("taskid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupNote(data::Database& db, const std::string& notedate, int activityId) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT noteid FROM ActivityNotes WHERE notedate = ? AND activityid = ?"
    );
    stmt->setString(1, notedate);
    stmt->setInt(2, activityId);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("noteid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupArtefactType(data::Database& db, const std::string& name, int projectId) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT artefacttypeid FROM ArtefactTypes WHERE artefactname = ? AND projectid = ?"
    );
    stmt->setString(1, name);
    stmt->setInt(2, projectId);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("artefacttypeid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupArtefact(data::Database& db, const std::string& name, int artefactTypeId) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT artefactid FROM Artefacts WHERE artefacttitle = ? AND artefacttypeid = ?"
    );
    stmt->setString(1, name);
    stmt->setInt(2, artefactTypeId);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("artefactid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupMilestone(data::Database& db, const std::string& name, int projectId) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT milestoneid FROM Milestones WHERE milestonename = ? AND projectid = ?"
    );
    stmt->setString(1, name);
    stmt->setInt(2, projectId);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("milestoneid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupAssignment(data::Database& db, const std::string& name, int taskId) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT ata.assignmentid FROM ActivityTaskAssignments ata "
        "JOIN Employees e ON ata.userid = e.employeeid "
        "WHERE (CONCAT(e.firstname, ' ', e.lastname) = ? "
        "   OR e.firstname = ? "
        "   OR e.lastname = ?) "
        "AND ata.taskid = ?"
    );
    stmt->setString(1, name);
    stmt->setString(2, name);
    stmt->setString(3, name);
    stmt->setInt(4, taskId);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("assignmentid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupArtefactActivityLink(data::Database& db, const std::string& name, int artefactId) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT aal.artefactlinkid FROM ArtefactToActivityLink aal "
        "JOIN Activities a ON aal.activityid = a.activityid "
        "WHERE a.activityname = ? AND aal.artefactid = ?"
    );
    stmt->setString(1, name);
    stmt->setInt(2, artefactId);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("artefactlinkid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupArtefactDataItem(data::Database& db, const std::string& fieldTitle, int artefactId) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT ad.artefactdataid FROM ArtefactData ad "
        "JOIN ArtefactDataFields adf ON ad.artefactdatafieldid = adf.artefactdatafieldid "
        "WHERE adf.fieldtitle = ? AND ad.artefactid = ?"
    );
    stmt->setString(1, fieldTitle);
    stmt->setInt(2, artefactId);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("artefactdataid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupArtefactDataField(data::Database& db, const std::string& name, int artefactTypeId) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT artefactdatafieldid FROM ArtefactDataFields WHERE fieldtitle = ? AND artefacttypeid = ?"
    );
    stmt->setString(1, name);
    stmt->setInt(2, artefactTypeId);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("artefactdatafieldid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupTeamMember(data::Database& db, const std::string& name, int projectId) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT ptm.teammemberid FROM ProjectTeamMembers ptm "
        "JOIN Employees e ON ptm.employeeid = e.employeeid "
        "WHERE CONCAT(e.firstname, ' ', e.lastname) = ? AND ptm.projectid = ?"
    );
    stmt->setString(1, name);
    stmt->setInt(2, projectId);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("teammemberid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupMilestoneToArtefactLink(data::Database& db, const std::string& name, int artefactLinkId) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT mal.milestonelinkid FROM MilestoneToArtefactLink mal "
        "JOIN MilestoneSteps ms ON mal.milestonestepid = ms.milestonestepid "
        "WHERE ms.milestonestepname = ? AND mal.artefactlinkid = ?"
    );
    stmt->setString(1, name);
    stmt->setInt(2, artefactLinkId);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("milestonelinkid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

LookupResult lookupHours(data::Database& db, const std::string& bookedDate, int assignmentId) {
    LookupResult result = {0, false, false};

    auto stmt = db.prepareStatement(
        "SELECT bookingid FROM Hours WHERE bookeddate = ? AND assignmentid = ?"
    );
    stmt->setString(1, bookedDate);
    stmt->setInt(2, assignmentId);
    auto res = stmt->executeQuery();

    if (res->next()) {
        result.id = res->getInt("bookingid");
        result.found = true;
        if (res->next()) {
            result.multipleMatches = true;
        }
    }
    return result;
}

std::vector<std::string> completionTeamMembers(data::Database& db, const std::string& prefix, int projectId) {
    std::vector<std::string> result;
    auto stmt = db.prepareStatement(
        "SELECT CONCAT(e.firstname, ' ', e.lastname) AS name "
        "FROM Employees e "
        "JOIN ProjectTeamMembers ptm ON e.employeeid = ptm.employeeid "
        "WHERE ptm.projectid = ? "
        "AND LOWER(CONCAT(e.firstname, ' ', e.lastname)) LIKE LOWER(?) "
        "LIMIT 50"
    );
    stmt->setInt(1, projectId);
    stmt->setString(2, prefix + "%");
    auto res = stmt->executeQuery();
    while (res->next()) {
        result.push_back(res->getString("name"));
    }
    return result;
}

std::vector<std::string> completionEmployees(data::Database& db, const std::string& prefix) {
    std::vector<std::string> result;
    auto stmt = db.prepareStatement(
        "SELECT CONCAT(firstname, ' ', lastname) AS name "
        "FROM Employees "
        "WHERE LOWER(CONCAT(firstname, ' ', lastname)) LIKE LOWER(?) "
        "LIMIT 50"
    );
    stmt->setString(1, prefix + "%");
    auto res = stmt->executeQuery();
    while (res->next()) {
        result.push_back(res->getString("name"));
    }
    return result;
}

std::vector<std::string> completionDepartments(data::Database& db, const std::string& prefix) {
    std::vector<std::string> result;
    auto stmt = db.prepareStatement(
        "SELECT departmentname AS name "
        "FROM Departments "
        "WHERE LOWER(departmentname) LIKE LOWER(?) "
        "LIMIT 50"
    );
    stmt->setString(1, prefix + "%");
    auto res = stmt->executeQuery();
    while (res->next()) {
        result.push_back(res->getString("name"));
    }
    return result;
}

std::vector<std::string> completionActivitiesByProject(data::Database& db, const std::string& prefix, int projectId) {
    std::vector<std::string> result;
    auto stmt = db.prepareStatement(
        "SELECT a.activityname AS name "
        "FROM Activities a "
        "JOIN ActivitySubcategories s ON a.subcategoryid = s.subcategoryid "
        "JOIN ActivityCategories c ON s.categoryid = c.categoryid "
        "WHERE c.projectid = ? "
        "AND LOWER(a.activityname) LIKE LOWER(?) "
        "LIMIT 50"
    );
    stmt->setInt(1, projectId);
    stmt->setString(2, prefix + "%");
    auto res = stmt->executeQuery();
    while (res->next()) {
        result.push_back(res->getString("name"));
    }
    return result;
}

std::vector<std::string> completionMilestoneStepsByProject(data::Database& db, const std::string& prefix, int projectId) {
    std::vector<std::string> result;
    auto stmt = db.prepareStatement(
        "SELECT ms.milestonestepname AS name "
        "FROM MilestoneSteps ms "
        "JOIN Milestones m ON ms.milestoneid = m.milestoneid "
        "WHERE m.projectid = ? "
        "AND LOWER(ms.milestonestepname) LIKE LOWER(?) "
        "LIMIT 50"
    );
    stmt->setInt(1, projectId);
    stmt->setString(2, prefix + "%");
    auto res = stmt->executeQuery();
    while (res->next()) {
        result.push_back(res->getString("name"));
    }
    return result;
}

int promptIdOrName(data::Database& db, const std::string& prompt,
                   std::function<LookupResult(data::Database&, const std::string&)> lookupFn,
                   std::function<std::vector<std::string>(const std::string&)> completionFn) {
    std::string input;
    if (completionFn) {
        input = readline_wrapper::readLineWithCompletion(prompt, completionFn);
    } else {
        input = utils::promptString(prompt);
    }

    // Try to parse as integer first
    try {
        int id = std::stoi(input);
        return id;
    } catch (const std::exception&) {
        // Not a number, try lookup by name
    }

    LookupResult result = lookupFn(db, input);

    if (result.multipleMatches) {
        throw ValidationError("Multiple items found with name \"" + input + "\". Please use ID instead.");
    }

    if (!result.found) {
        throw ValidationError("No item found with name \"" + input + "\"");
    }

    return result.id;
}

int promptIdOrNameScoped(data::Database& db, const std::string& prompt, int scopeId,
                         std::function<LookupResult(data::Database&, const std::string&, int)> lookupFn,
                         std::function<std::vector<std::string>(const std::string&)> completionFn) {
    std::string input;
    if (completionFn) {
        input = readline_wrapper::readLineWithCompletion(prompt, completionFn);
    } else {
        input = utils::promptString(prompt);
    }

    // Try to parse as integer first
    try {
        int id = std::stoi(input);
        return id;
    } catch (const std::exception&) {
        // Not a number, try lookup by name
    }

    LookupResult result = lookupFn(db, input, scopeId);

    if (result.multipleMatches) {
        throw ValidationError("Multiple items found with name \"" + input + "\". Please use ID instead.");
    }

    if (!result.found) {
        throw ValidationError("No item found with name \"" + input + "\"");
    }

    return result.id;
}

int resolveIdOrName(data::Database& db, const std::string& value,
                    std::function<LookupResult(data::Database&, const std::string&)> lookupFn) {
    // Try to parse as integer first
    try {
        int id = std::stoi(value);
        return id;
    } catch (const std::exception&) {
        // Not a number, try lookup by name
    }

    LookupResult result = lookupFn(db, value);

    if (result.multipleMatches) {
        throw ValidationError("Multiple items found with name \"" + value + "\". Please use ID instead.");
    }

    if (!result.found) {
        throw ValidationError("No item found with name \"" + value + "\"");
    }

    return result.id;
}

int resolveIdOrNameScoped(data::Database& db, const std::string& value, int scopeId,
                          std::function<LookupResult(data::Database&, const std::string&, int)> lookupFn) {
    // Try to parse as integer first
    try {
        int id = std::stoi(value);
        return id;
    } catch (const std::exception&) {
        // Not a number, try lookup by name
    }

    LookupResult result = lookupFn(db, value, scopeId);

    if (result.multipleMatches) {
        throw ValidationError("Multiple items found with name \"" + value + "\". Please use ID instead.");
    }

    if (!result.found) {
        throw ValidationError("No item found with name \"" + value + "\"");
    }

    return result.id;
}

} // namespace utils
