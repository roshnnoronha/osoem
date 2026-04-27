#include "completion_provider.h"
#include "../utils/utils.h"
#include <algorithm>
#include <cctype>

CompletionProvider::CompletionProvider(Database& db, Path& path)
    : db(db), path(path) {}

std::string CompletionProvider::quoteIfNeeded(const std::string& name) {
    if (name.find(' ') != std::string::npos) {
        return "\"" + name + "\"";
    }
    return name;
}

// Insert '/' before closing quote for quoted strings, or append for bare strings
static void appendSlash(std::string& s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        s.insert(s.end() - 1, '/');
    } else {
        s += '/';
    }
}

// Returns true when completions at this context type lead to a navigable (non-leaf) node
static bool completionDestIsNavigable(Path::ItemType fromType) {
    switch (fromType) {
    case Path::DEPARTMENT:                // → EMPLOYEE (leaf)
    case Path::TEAM_FOLDER:               // → PROJECT_TEAM_MEMBER (leaf)
    case Path::ASSIGNMENT:                // → HOURS (leaf)
    case Path::NOTES_FOLDER:              // → ACTIVITY_NOTE (leaf)
    case Path::DATA_FOLDER:               // → ARTEFACT_DATA_ITEM (leaf)
    case Path::ARTEFACT_TO_ACTIVITY_LINK: // → MILESTONE_TO_ARTEFACT_LINK (leaf)
    case Path::FIELDS_FOLDER:             // → ARTEFACT_FIELD (leaf)
    case Path::MILESTONE:                 // → MILESTONE_STEP (leaf)
        return false;
    default:
        return true;
    }
}

std::vector<std::string> CompletionProvider::getCompletions(const std::string& prefix) {
    Path::ItemType currentType = path.current_type();
    std::vector<std::string> results;

    switch (currentType) {
        case Path::ROOT:
            results = getRootCompletions(prefix); break;
        case Path::ORGANIZATION_FOLDER:
            results = getOrganizationFolderCompletions(prefix); break;
        case Path::DEPARTMENTS_FOLDER:
            results = getDepartmentCompletions(prefix); break;
        case Path::DEPARTMENT:
            results = getDepartmentEmployeeCompletions(prefix, path.current_id()); break;
        case Path::PROJECTS_FOLDER:
            results = getProjectCompletions(prefix); break;
        case Path::PROJECT:
            results = getProjectFolderCompletions(prefix); break;
        case Path::TEAM_FOLDER:
            results = getTeamMemberCompletions(prefix, path.getIdByType(Path::PROJECT)); break;
        case Path::ACTIVITIES_FOLDER:
            results = getCategoryCompletions(prefix, path.getIdByType(Path::PROJECT)); break;
        case Path::CATEGORY:
            results = getSubcategoryCompletions(prefix, path.current_id()); break;
        case Path::SUBCATEGORY:
            results = getActivityCompletions(prefix, path.current_id()); break;
        case Path::ACTIVITY:
            results = getActivityFolderCompletions(prefix); break;
        case Path::TASKS_FOLDER:
            if (path.getIdByType(Path::TASK) > 0)
                results = getSubtaskCompletions(prefix, path.current_id());
            else
                results = getTaskCompletions(prefix, path.current_id());
            break;
        case Path::TASK:
        case Path::SUBTASK:
            results = getTaskFolderCompletions(prefix); break;
        case Path::ASSIGNMENTS_FOLDER:
            results = getAssignmentCompletions(prefix, path.current_id()); break;
        case Path::ASSIGNMENT:
            results = getHoursCompletions(prefix, path.current_id()); break;
        case Path::NOTES_FOLDER:
            results = getNoteCompletions(prefix, path.getIdByType(Path::ACTIVITY)); break;
        case Path::ARTEFACT_TYPES_FOLDER:
            results = getArtefactTypeCompletions(prefix, path.getIdByType(Path::PROJECT)); break;
        case Path::ARTEFACT_TYPE:
            results = getArtefactTypeFolderCompletions(prefix); break;
        case Path::ARTEFACTS_FOLDER:
            results = getArtefactCompletions(prefix, path.getIdByType(Path::ARTEFACT_TYPE)); break;
        case Path::ARTEFACT:
            results = getArtefactFolderCompletions(prefix); break;
        case Path::DATA_FOLDER:
            results = getArtefactDataItemCompletions(prefix, path.getIdByType(Path::ARTEFACT)); break;
        case Path::ASSOCIATIONS_FOLDER:
            results = getArtefactToActivityLinkCompletions(prefix, path.getIdByType(Path::ARTEFACT)); break;
        case Path::ARTEFACT_TO_ACTIVITY_LINK:
            results = getMilestoneToArtefactLinkCompletions(prefix, path.current_id()); break;
        case Path::FIELDS_FOLDER:
            results = getFieldCompletions(prefix, path.getIdByType(Path::ARTEFACT_TYPE)); break;
        case Path::MILESTONES_FOLDER:
            results = getMilestoneCompletions(prefix, path.getIdByType(Path::PROJECT)); break;
        case Path::MILESTONE:
            results = getMilestoneStepCompletions(prefix, path.current_id()); break;
        default:
            break;
    }

    if (completionDestIsNavigable(currentType)) {
        for (auto& s : results) appendSlash(s);
    }
    return results;
}

std::vector<std::string> CompletionProvider::getCommandCompletions(const std::string& prefix) {
    std::vector<std::string> commands = {
        "ls", "list", "cd", "sl", "select", "ad", "add",
        "rm", "remove", "st", "set", "im", "import",
        "ex", "export", "so", "source", "exit", "quit"
    };

    std::vector<std::string> matches;
    std::string lowerPrefix = prefix;
    std::transform(lowerPrefix.begin(), lowerPrefix.end(), lowerPrefix.begin(), ::tolower);

    for (const auto& cmd : commands) {
        if (cmd.find(lowerPrefix) == 0) {
            matches.push_back(cmd);
        }
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getRootCompletions(const std::string& prefix) {
    std::vector<std::string> folders = {"Organization", "Projects"};
    std::vector<std::string> matches;

    std::string lowerPrefix = prefix;
    std::transform(lowerPrefix.begin(), lowerPrefix.end(), lowerPrefix.begin(), ::tolower);

    for (const auto& folder : folders) {
        std::string lowerFolder = folder;
        std::transform(lowerFolder.begin(), lowerFolder.end(), lowerFolder.begin(), ::tolower);
        if (lowerFolder.find(lowerPrefix) == 0) {
            matches.push_back(folder);
        }
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getEmployeeCompletions(const std::string& prefix) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT employeeid, CONCAT(firstname, ' ', lastname) AS name FROM Employees "
            "WHERE LOWER(CONCAT(firstname, ' ', lastname)) LIKE LOWER(?) "
            "OR CAST(employeeid AS CHAR) LIKE ? "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setString(1, prefix + "%");
        stmt->setString(2, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            matches.push_back(quoteIfNeeded(res->getString("name")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail - return empty matches
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getProjectCompletions(const std::string& prefix) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT projectid, projectname FROM Projects "
            "WHERE LOWER(projectname) LIKE LOWER(?) "
            "OR CAST(projectid AS CHAR) LIKE ? "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setString(1, prefix + "%");
        stmt->setString(2, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            matches.push_back(quoteIfNeeded(res->getString("projectname")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getProjectFolderCompletions(const std::string& prefix) {
    std::vector<std::string> folders = {"Activity Categories", "Artefact Types", "Milestones", "Team"};
    std::vector<std::string> matches;

    std::string lowerPrefix = prefix;
    std::transform(lowerPrefix.begin(), lowerPrefix.end(), lowerPrefix.begin(), ::tolower);

    for (const auto& folder : folders) {
        std::string lowerFolder = folder;
        std::transform(lowerFolder.begin(), lowerFolder.end(), lowerFolder.begin(), ::tolower);
        if (lowerFolder.find(lowerPrefix) == 0) {
            matches.push_back(quoteIfNeeded(folder));
        }
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getCategoryCompletions(const std::string& prefix, int projectId) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT categoryid, categoryname FROM ActivityCategories "
            "WHERE projectid = ? AND (LOWER(categoryname) LIKE LOWER(?) "
            "OR CAST(categoryid AS CHAR) LIKE ?) "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, projectId);
        stmt->setString(2, prefix + "%");
        stmt->setString(3, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            matches.push_back(quoteIfNeeded(res->getString("categoryname")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getSubcategoryCompletions(const std::string& prefix, int categoryId) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT subcategoryid, subcategoryname FROM ActivitySubcategories "
            "WHERE categoryid = ? AND (LOWER(subcategoryname) LIKE LOWER(?) "
            "OR CAST(subcategoryid AS CHAR) LIKE ?) "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, categoryId);
        stmt->setString(2, prefix + "%");
        stmt->setString(3, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            matches.push_back(quoteIfNeeded(res->getString("subcategoryname")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getActivityCompletions(const std::string& prefix, int subcategoryId) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT activityid, activityname FROM Activities "
            "WHERE subcategoryid = ? AND (LOWER(activityname) LIKE LOWER(?) "
            "OR CAST(activityid AS CHAR) LIKE ?) "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, subcategoryId);
        stmt->setString(2, prefix + "%");
        stmt->setString(3, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            matches.push_back(quoteIfNeeded(res->getString("activityname")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getActivityFolderCompletions(const std::string& prefix) {
    std::vector<std::string> folders = {"Tasks", "Notes"};
    std::vector<std::string> matches;

    std::string lowerPrefix = prefix;
    std::transform(lowerPrefix.begin(), lowerPrefix.end(), lowerPrefix.begin(), ::tolower);

    for (const auto& folder : folders) {
        std::string lowerFolder = folder;
        std::transform(lowerFolder.begin(), lowerFolder.end(), lowerFolder.begin(), ::tolower);
        if (lowerFolder.find(lowerPrefix) == 0) {
            matches.push_back(folder);
        }
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getTaskCompletions(const std::string& prefix, int activityId) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT taskid, taskname FROM ActivityTasks "
            "WHERE activityid = ? AND parenttaskid IS NULL "
            "AND (LOWER(taskname) LIKE LOWER(?) OR CAST(taskid AS CHAR) LIKE ?) "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, activityId);
        stmt->setString(2, prefix + "%");
        stmt->setString(3, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            matches.push_back(quoteIfNeeded(res->getString("taskname")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getSubtaskCompletions(const std::string& prefix, int parentTaskId) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT taskid, taskname FROM ActivityTasks "
            "WHERE parenttaskid = ? "
            "AND (LOWER(taskname) LIKE LOWER(?) OR CAST(taskid AS CHAR) LIKE ?) "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, parentTaskId);
        stmt->setString(2, prefix + "%");
        stmt->setString(3, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            matches.push_back(quoteIfNeeded(res->getString("taskname")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getNoteCompletions(const std::string& prefix, int activityId) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT noteid, LEFT(note, 50) AS note_preview FROM ActivityNotes "
            "WHERE activityid = ? "
            "AND (LOWER(note) LIKE LOWER(?) OR CAST(noteid AS CHAR) LIKE ?) "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, activityId);
        stmt->setString(2, prefix + "%");
        stmt->setString(3, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            std::string notePreview = res->getString("note_preview");
            // Use note ID for completion since notes might not have unique names
            matches.push_back(std::to_string(res->getInt("noteid")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getArtefactTypeCompletions(const std::string& prefix, int projectId) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT artefacttypeid, artefactname FROM ArtefactTypes "
            "WHERE projectid = ? AND (LOWER(artefactname) LIKE LOWER(?) "
            "OR CAST(artefacttypeid AS CHAR) LIKE ?) "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, projectId);
        stmt->setString(2, prefix + "%");
        stmt->setString(3, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            matches.push_back(quoteIfNeeded(res->getString("artefactname")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getArtefactTypeFolderCompletions(const std::string& prefix) {
    std::vector<std::string> folders = {"Artefacts", "Fields"};
    std::vector<std::string> matches;

    std::string lowerPrefix = prefix;
    std::transform(lowerPrefix.begin(), lowerPrefix.end(), lowerPrefix.begin(), ::tolower);

    for (const auto& folder : folders) {
        std::string lowerFolder = folder;
        std::transform(lowerFolder.begin(), lowerFolder.end(), lowerFolder.begin(), ::tolower);
        if (lowerFolder.find(lowerPrefix) == 0) {
            matches.push_back(folder);
        }
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getArtefactCompletions(const std::string& prefix, int artefactTypeId) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT artefactid, artefacttitle FROM Artefacts "
            "WHERE artefacttypeid = ? AND (LOWER(artefacttitle) LIKE LOWER(?) "
            "OR CAST(artefactid AS CHAR) LIKE ?) "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, artefactTypeId);
        stmt->setString(2, prefix + "%");
        stmt->setString(3, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            matches.push_back(quoteIfNeeded(res->getString("artefacttitle")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getFieldCompletions(const std::string& prefix, int artefactTypeId) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT artefactdatafieldid, fieldtitle FROM ArtefactDataFields "
            "WHERE artefacttypeid = ? AND (LOWER(fieldtitle) LIKE LOWER(?) "
            "OR CAST(artefactdatafieldid AS CHAR) LIKE ?) "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, artefactTypeId);
        stmt->setString(2, prefix + "%");
        stmt->setString(3, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            matches.push_back(quoteIfNeeded(res->getString("fieldtitle")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getMilestoneCompletions(const std::string& prefix, int projectId) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT milestoneid, milestonename FROM Milestones "
            "WHERE projectid = ? AND (LOWER(milestonename) LIKE LOWER(?) "
            "OR CAST(milestoneid AS CHAR) LIKE ?) "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, projectId);
        stmt->setString(2, prefix + "%");
        stmt->setString(3, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            matches.push_back(quoteIfNeeded(res->getString("milestonename")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getMilestoneStepCompletions(const std::string& prefix, int milestoneId) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT milestonestepid, milestonestepname FROM MilestoneSteps "
            "WHERE milestoneid = ? AND (LOWER(milestonestepname) LIKE LOWER(?) "
            "OR CAST(milestonestepid AS CHAR) LIKE ?) "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, milestoneId);
        stmt->setString(2, prefix + "%");
        stmt->setString(3, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            matches.push_back(quoteIfNeeded(res->getString("milestonestepname")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getOrganizationFolderCompletions(const std::string& prefix) {
    std::vector<std::string> folders = {"Departments"};
    std::vector<std::string> matches;

    std::string lowerPrefix = prefix;
    std::transform(lowerPrefix.begin(), lowerPrefix.end(), lowerPrefix.begin(), ::tolower);

    for (const auto& folder : folders) {
        std::string lowerFolder = folder;
        std::transform(lowerFolder.begin(), lowerFolder.end(), lowerFolder.begin(), ::tolower);
        if (lowerFolder.find(lowerPrefix) == 0) {
            matches.push_back(folder);
        }
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getDepartmentCompletions(const std::string& prefix) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT departmentid, departmentname FROM Departments "
            "WHERE LOWER(departmentname) LIKE LOWER(?) "
            "OR CAST(departmentid AS CHAR) LIKE ? "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setString(1, prefix + "%");
        stmt->setString(2, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            matches.push_back(quoteIfNeeded(res->getString("departmentname")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getDepartmentEmployeeCompletions(const std::string& prefix, int departmentId) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT employeeid, CONCAT(firstname, ' ', lastname) AS name FROM Employees "
            "WHERE departmentid = ? AND (LOWER(CONCAT(firstname, ' ', lastname)) LIKE LOWER(?) "
            "OR CAST(employeeid AS CHAR) LIKE ?) "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, departmentId);
        stmt->setString(2, prefix + "%");
        stmt->setString(3, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            matches.push_back(quoteIfNeeded(res->getString("name")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getTeamMemberCompletions(const std::string& prefix, int projectId) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT ptm.teammemberid, CONCAT(e.firstname, ' ', e.lastname) AS name "
            "FROM ProjectTeamMembers ptm "
            "JOIN Employees e ON ptm.employeeid = e.employeeid "
            "WHERE ptm.projectid = ? AND (LOWER(CONCAT(e.firstname, ' ', e.lastname)) LIKE LOWER(?) "
            "OR CAST(ptm.teammemberid AS CHAR) LIKE ?) "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, projectId);
        stmt->setString(2, prefix + "%");
        stmt->setString(3, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            matches.push_back(quoteIfNeeded(res->getString("name")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getTaskFolderCompletions(const std::string& prefix) {
    std::vector<std::string> folders = {"Tasks", "Assignments"};
    std::vector<std::string> matches;

    std::string lowerPrefix = prefix;
    std::transform(lowerPrefix.begin(), lowerPrefix.end(), lowerPrefix.begin(), ::tolower);

    for (const auto& folder : folders) {
        std::string lowerFolder = folder;
        std::transform(lowerFolder.begin(), lowerFolder.end(), lowerFolder.begin(), ::tolower);
        if (lowerFolder.find(lowerPrefix) == 0) {
            matches.push_back(folder);
        }
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getAssignmentCompletions(const std::string& prefix, int taskId) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT ata.assignmentid, CONCAT(e.firstname, ' ', e.lastname) AS name "
            "FROM ActivityTaskAssignments ata "
            "JOIN Employees e ON ata.userid = e.employeeid "
            "WHERE ata.taskid = ? AND (LOWER(CONCAT(e.firstname, ' ', e.lastname)) LIKE LOWER(?) "
            "OR CAST(ata.assignmentid AS CHAR) LIKE ?) "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, taskId);
        stmt->setString(2, prefix + "%");
        stmt->setString(3, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            matches.push_back(quoteIfNeeded(res->getString("name")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getHoursCompletions(const std::string& prefix, int assignmentId) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT bookingid, bookeddate FROM Hours "
            "WHERE assignmentid = ? AND (CAST(bookingid AS CHAR) LIKE ? "
            "OR bookeddate LIKE ?) "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, assignmentId);
        stmt->setString(2, prefix + "%");
        stmt->setString(3, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            // Use booking ID for completion
            matches.push_back(std::to_string(res->getInt("bookingid")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getArtefactFolderCompletions(const std::string& prefix) {
    std::vector<std::string> folders = {"Data", "Associations"};
    std::vector<std::string> matches;

    std::string lowerPrefix = prefix;
    std::transform(lowerPrefix.begin(), lowerPrefix.end(), lowerPrefix.begin(), ::tolower);

    for (const auto& folder : folders) {
        std::string lowerFolder = folder;
        std::transform(lowerFolder.begin(), lowerFolder.end(), lowerFolder.begin(), ::tolower);
        if (lowerFolder.find(lowerPrefix) == 0) {
            matches.push_back(folder);
        }
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getArtefactDataItemCompletions(const std::string& prefix, int artefactId) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT ad.artefactdataid, adf.fieldtitle FROM ArtefactData ad "
            "JOIN ArtefactDataFields adf ON ad.artefactdatafieldid = adf.artefactdatafieldid "
            "WHERE ad.artefactid = ? AND (LOWER(adf.fieldtitle) LIKE LOWER(?) "
            "OR CAST(ad.artefactdataid AS CHAR) LIKE ?) "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, artefactId);
        stmt->setString(2, prefix + "%");
        stmt->setString(3, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            matches.push_back(quoteIfNeeded(res->getString("fieldtitle")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getArtefactToActivityLinkCompletions(const std::string& prefix, int artefactId) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT aal.artefactlinkid, a.activityname FROM ArtefactToActivityLink aal "
            "JOIN Activities a ON aal.activityid = a.activityid "
            "WHERE aal.artefactid = ? AND (LOWER(a.activityname) LIKE LOWER(?) "
            "OR CAST(aal.artefactlinkid AS CHAR) LIKE ?) "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, artefactId);
        stmt->setString(2, prefix + "%");
        stmt->setString(3, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            matches.push_back(quoteIfNeeded(res->getString("activityname")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail
    }
    return matches;
}

std::vector<std::string> CompletionProvider::getMilestoneToArtefactLinkCompletions(const std::string& prefix, int artefactLinkId) {
    std::vector<std::string> matches;
    try {
        std::string query =
            "SELECT mal.milestonelinkid, ms.milestonestepname FROM MilestoneToArtefactLink mal "
            "JOIN MilestoneSteps ms ON mal.milestonestepid = ms.milestonestepid "
            "WHERE mal.artefactlinkid = ? AND (LOWER(ms.milestonestepname) LIKE LOWER(?) "
            "OR CAST(mal.milestonelinkid AS CHAR) LIKE ?) "
            "LIMIT 50";

        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, artefactLinkId);
        stmt->setString(2, prefix + "%");
        stmt->setString(3, prefix + "%");
        auto res = stmt->executeQuery();

        while (res->next()) {
            matches.push_back(quoteIfNeeded(res->getString("milestonestepname")));
        }
    } catch (sql::SQLException& e) {
        // Silently fail
    }
    return matches;
}

// =============================================================================
// Path-based completion helpers
// =============================================================================

int CompletionProvider::getIdInSim(const SimStack& stack, Path::ItemType type) const {
    for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
        if (it->type == type) return it->id;
    }
    return -1;
}

bool CompletionProvider::navigateSimForward(SimStack& stack, const std::string& segment) {
    if (stack.empty()) return false;
    Path::ItemType cur = stack.back().type;
    int curId = stack.back().id;
    std::string lower = segment;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    // --- Folder navigation (no DB query) ---
    if (cur == Path::ROOT) {
        if (lower == "organization") { stack.push_back({Path::ORGANIZATION_FOLDER, 1}); return true; }
        if (lower == "projects")     { stack.push_back({Path::PROJECTS_FOLDER, 2});      return true; }
    }
    if (cur == Path::ORGANIZATION_FOLDER) {
        if (lower == "departments") { stack.push_back({Path::DEPARTMENTS_FOLDER, 1}); return true; }
    }
    if (cur == Path::PROJECT) {
        if (lower == "activity categories" || lower == "activitycategories")
            { stack.push_back({Path::ACTIVITIES_FOLDER, curId}); return true; }
        if (lower == "artefact types" || lower == "artefacttypes")
            { stack.push_back({Path::ARTEFACT_TYPES_FOLDER, curId}); return true; }
        if (lower == "milestones") { stack.push_back({Path::MILESTONES_FOLDER, curId}); return true; }
        if (lower == "team")       { stack.push_back({Path::TEAM_FOLDER, curId});       return true; }
    }
    if (cur == Path::ACTIVITY) {
        if (lower == "tasks") { stack.push_back({Path::TASKS_FOLDER, curId}); return true; }
        if (lower == "notes") { stack.push_back({Path::NOTES_FOLDER, curId}); return true; }
    }
    if (cur == Path::TASK || cur == Path::SUBTASK) {
        if (lower == "tasks" || lower == "subtasks")
            { stack.push_back({Path::TASKS_FOLDER, curId}); return true; }
        if (lower == "assignments")
            { stack.push_back({Path::ASSIGNMENTS_FOLDER, curId}); return true; }
    }
    if (cur == Path::ARTEFACT_TYPE) {
        if (lower == "artefacts") { stack.push_back({Path::ARTEFACTS_FOLDER, curId}); return true; }
        if (lower == "fields")    { stack.push_back({Path::FIELDS_FOLDER, curId});    return true; }
    }
    if (cur == Path::ARTEFACT) {
        if (lower == "data")         { stack.push_back({Path::DATA_FOLDER, curId});         return true; }
        if (lower == "associations") { stack.push_back({Path::ASSOCIATIONS_FOLDER, curId}); return true; }
    }

    // --- Entity navigation (exact-match DB query) ---
    try {
        switch (cur) {
        case Path::DEPARTMENTS_FOLDER: {
            auto stmt = db.prepareStatement(
                "SELECT departmentid FROM Departments WHERE LOWER(departmentname) = LOWER(?) LIMIT 1");
            stmt->setString(1, segment);
            auto res = stmt->executeQuery();
            if (res->next()) { stack.push_back({Path::DEPARTMENT, res->getInt("departmentid")}); return true; }
            break;
        }
        case Path::PROJECTS_FOLDER: {
            auto stmt = db.prepareStatement(
                "SELECT projectid FROM Projects WHERE LOWER(projectname) = LOWER(?) LIMIT 1");
            stmt->setString(1, segment);
            auto res = stmt->executeQuery();
            if (res->next()) { stack.push_back({Path::PROJECT, res->getInt("projectid")}); return true; }
            break;
        }
        case Path::ACTIVITIES_FOLDER: {
            // curId = projectId (stored in ACTIVITIES_FOLDER)
            auto stmt = db.prepareStatement(
                "SELECT categoryid FROM ActivityCategories WHERE LOWER(categoryname) = LOWER(?) AND projectid = ? LIMIT 1");
            stmt->setString(1, segment);
            stmt->setInt(2, curId);
            auto res = stmt->executeQuery();
            if (res->next()) { stack.push_back({Path::CATEGORY, res->getInt("categoryid")}); return true; }
            break;
        }
        case Path::CATEGORY: {
            auto stmt = db.prepareStatement(
                "SELECT subcategoryid FROM ActivitySubcategories WHERE LOWER(subcategoryname) = LOWER(?) AND categoryid = ? LIMIT 1");
            stmt->setString(1, segment);
            stmt->setInt(2, curId);
            auto res = stmt->executeQuery();
            if (res->next()) { stack.push_back({Path::SUBCATEGORY, res->getInt("subcategoryid")}); return true; }
            break;
        }
        case Path::SUBCATEGORY: {
            auto stmt = db.prepareStatement(
                "SELECT activityid FROM Activities WHERE LOWER(activityname) = LOWER(?) AND subcategoryid = ? LIMIT 1");
            stmt->setString(1, segment);
            stmt->setInt(2, curId);
            auto res = stmt->executeQuery();
            if (res->next()) { stack.push_back({Path::ACTIVITY, res->getInt("activityid")}); return true; }
            break;
        }
        case Path::TASKS_FOLDER: {
            if (getIdInSim(stack, Path::TASK) > 0) {
                // Subtask: curId = parent task id
                auto stmt = db.prepareStatement(
                    "SELECT taskid FROM ActivityTasks WHERE LOWER(taskname) = LOWER(?) AND parenttaskid = ? LIMIT 1");
                stmt->setString(1, segment);
                stmt->setInt(2, curId);
                auto res = stmt->executeQuery();
                if (res->next()) { stack.push_back({Path::TASK, res->getInt("taskid")}); return true; }
            } else {
                // Top-level task: curId = activityId
                auto stmt = db.prepareStatement(
                    "SELECT taskid FROM ActivityTasks WHERE LOWER(taskname) = LOWER(?) AND activityid = ? AND parenttaskid IS NULL LIMIT 1");
                stmt->setString(1, segment);
                stmt->setInt(2, curId);
                auto res = stmt->executeQuery();
                if (res->next()) { stack.push_back({Path::TASK, res->getInt("taskid")}); return true; }
            }
            break;
        }
        case Path::ASSIGNMENTS_FOLDER: {
            // curId = taskId
            auto stmt = db.prepareStatement(
                "SELECT ata.assignmentid FROM ActivityTaskAssignments ata "
                "JOIN Employees e ON ata.userid = e.employeeid "
                "WHERE (LOWER(CONCAT(e.firstname, ' ', e.lastname)) = LOWER(?) "
                "   OR LOWER(e.firstname) = LOWER(?) OR LOWER(e.lastname) = LOWER(?)) "
                "AND ata.taskid = ? LIMIT 1");
            stmt->setString(1, segment);
            stmt->setString(2, segment);
            stmt->setString(3, segment);
            stmt->setInt(4, curId);
            auto res = stmt->executeQuery();
            if (res->next()) { stack.push_back({Path::ASSIGNMENT, res->getInt("assignmentid")}); return true; }
            break;
        }
        case Path::ARTEFACT_TYPES_FOLDER: {
            // curId = projectId
            auto stmt = db.prepareStatement(
                "SELECT artefacttypeid FROM ArtefactTypes WHERE LOWER(artefactname) = LOWER(?) AND projectid = ? LIMIT 1");
            stmt->setString(1, segment);
            stmt->setInt(2, curId);
            auto res = stmt->executeQuery();
            if (res->next()) { stack.push_back({Path::ARTEFACT_TYPE, res->getInt("artefacttypeid")}); return true; }
            break;
        }
        case Path::ARTEFACTS_FOLDER: {
            // curId = artefactTypeId
            auto stmt = db.prepareStatement(
                "SELECT artefactid FROM Artefacts WHERE LOWER(artefacttitle) = LOWER(?) AND artefacttypeid = ? LIMIT 1");
            stmt->setString(1, segment);
            stmt->setInt(2, curId);
            auto res = stmt->executeQuery();
            if (res->next()) { stack.push_back({Path::ARTEFACT, res->getInt("artefactid")}); return true; }
            break;
        }
        case Path::ASSOCIATIONS_FOLDER: {
            // curId = artefactId
            auto stmt = db.prepareStatement(
                "SELECT aal.artefactlinkid FROM ArtefactToActivityLink aal "
                "JOIN Activities a ON aal.activityid = a.activityid "
                "WHERE LOWER(a.activityname) = LOWER(?) AND aal.artefactid = ? LIMIT 1");
            stmt->setString(1, segment);
            stmt->setInt(2, curId);
            auto res = stmt->executeQuery();
            if (res->next()) { stack.push_back({Path::ARTEFACT_TO_ACTIVITY_LINK, res->getInt("artefactlinkid")}); return true; }
            break;
        }
        case Path::MILESTONES_FOLDER: {
            // curId = projectId
            auto stmt = db.prepareStatement(
                "SELECT milestoneid FROM Milestones WHERE LOWER(milestonename) = LOWER(?) AND projectid = ? LIMIT 1");
            stmt->setString(1, segment);
            stmt->setInt(2, curId);
            auto res = stmt->executeQuery();
            if (res->next()) { stack.push_back({Path::MILESTONE, res->getInt("milestoneid")}); return true; }
            break;
        }
        case Path::MILESTONE: {
            auto stmt = db.prepareStatement(
                "SELECT milestonestepid FROM MilestoneSteps WHERE LOWER(milestonestepname) = LOWER(?) AND milestoneid = ? LIMIT 1");
            stmt->setString(1, segment);
            stmt->setInt(2, curId);
            auto res = stmt->executeQuery();
            if (res->next()) { stack.push_back({Path::MILESTONE_STEP, res->getInt("milestonestepid")}); return true; }
            break;
        }
        case Path::FIELDS_FOLDER: {
            // curId = artefactTypeId
            auto stmt = db.prepareStatement(
                "SELECT artefactdatafieldid FROM ArtefactDataFields WHERE LOWER(fieldtitle) = LOWER(?) AND artefacttypeid = ? LIMIT 1");
            stmt->setString(1, segment);
            stmt->setInt(2, curId);
            auto res = stmt->executeQuery();
            if (res->next()) { stack.push_back({Path::ARTEFACT_FIELD, res->getInt("artefactdatafieldid")}); return true; }
            break;
        }
        default:
            break;
        }
    } catch (...) {}
    return false;
}

std::vector<std::string> CompletionProvider::getCompletionsForSim(const SimStack& stack, const std::string& prefix) {
    if (stack.empty()) return {};
    Path::ItemType type = stack.back().type;
    int id = stack.back().id;
    std::vector<std::string> results;

    switch (type) {
    case Path::ROOT:                      results = getRootCompletions(prefix); break;
    case Path::ORGANIZATION_FOLDER:       results = getOrganizationFolderCompletions(prefix); break;
    case Path::DEPARTMENTS_FOLDER:        results = getDepartmentCompletions(prefix); break;
    case Path::DEPARTMENT:                results = getDepartmentEmployeeCompletions(prefix, id); break;
    case Path::PROJECTS_FOLDER:           results = getProjectCompletions(prefix); break;
    case Path::PROJECT:                   results = getProjectFolderCompletions(prefix); break;
    case Path::TEAM_FOLDER:               results = getTeamMemberCompletions(prefix, id); break;
    case Path::ACTIVITIES_FOLDER:         results = getCategoryCompletions(prefix, id); break;
    case Path::CATEGORY:                  results = getSubcategoryCompletions(prefix, id); break;
    case Path::SUBCATEGORY:               results = getActivityCompletions(prefix, id); break;
    case Path::ACTIVITY:                  results = getActivityFolderCompletions(prefix); break;
    case Path::TASKS_FOLDER:
        if (getIdInSim(stack, Path::TASK) > 0)
            results = getSubtaskCompletions(prefix, id);
        else
            results = getTaskCompletions(prefix, id);
        break;
    case Path::TASK:
    case Path::SUBTASK:                   results = getTaskFolderCompletions(prefix); break;
    case Path::ASSIGNMENTS_FOLDER:        results = getAssignmentCompletions(prefix, id); break;
    case Path::ASSIGNMENT:                results = getHoursCompletions(prefix, id); break;
    case Path::NOTES_FOLDER:              results = getNoteCompletions(prefix, id); break;
    case Path::ARTEFACT_TYPES_FOLDER:     results = getArtefactTypeCompletions(prefix, id); break;
    case Path::ARTEFACT_TYPE:             results = getArtefactTypeFolderCompletions(prefix); break;
    case Path::ARTEFACTS_FOLDER:          results = getArtefactCompletions(prefix, id); break;
    case Path::ARTEFACT:                  results = getArtefactFolderCompletions(prefix); break;
    case Path::DATA_FOLDER:               results = getArtefactDataItemCompletions(prefix, id); break;
    case Path::ASSOCIATIONS_FOLDER:       results = getArtefactToActivityLinkCompletions(prefix, id); break;
    case Path::ARTEFACT_TO_ACTIVITY_LINK: results = getMilestoneToArtefactLinkCompletions(prefix, id); break;
    case Path::FIELDS_FOLDER:             results = getFieldCompletions(prefix, id); break;
    case Path::MILESTONES_FOLDER:         results = getMilestoneCompletions(prefix, id); break;
    case Path::MILESTONE:                 results = getMilestoneStepCompletions(prefix, id); break;
    default:                              break;
    }

    if (completionDestIsNavigable(type)) {
        for (auto& s : results) appendSlash(s);
    }
    return results;
}

std::vector<std::string> CompletionProvider::getPathCompletions(const std::string& fullPrefix) {
    // No '/' → fall through to single-level completion at current location
    if (fullPrefix.find('/') == std::string::npos) {
        return getCompletions(fullPrefix);
    }

    // Split into the resolved prefix (up to and including last '/') and the partial last segment
    size_t lastSlash = fullPrefix.rfind('/');
    std::string resolvedPart = fullPrefix.substr(0, lastSlash + 1); // e.g. "~/projects/"
    std::string lastSeg      = fullPrefix.substr(lastSlash + 1);    // e.g. "MyPro"
    std::string navPart      = fullPrefix.substr(0, lastSlash);     // without trailing '/'

    // Build simulated navigation stack
    SimStack simStack;
    auto navSegments = utils::split(navPart, '/');
    int startIdx = 0;

    if (!navSegments.empty() && navSegments[0] == "~") {
        // Absolute path: start fresh from ROOT
        simStack.push_back({Path::ROOT, -1});
        startIdx = 1;
    } else {
        // Relative path: start from current position
        simStack.push_back({path.current_type(), path.current_id()});
    }

    // Navigate through each resolved segment
    for (int i = startIdx; i < static_cast<int>(navSegments.size()); i++) {
        const std::string& seg = navSegments[i];
        if (seg.empty() || seg == ".") continue;
        if (seg == "..") {
            if (simStack.size() > 1) simStack.pop_back();
            continue;
        }
        if (!navigateSimForward(simStack, seg)) {
            return {}; // Can't resolve segment — no completions
        }
    }

    // Get raw completions for the partial last segment
    auto rawCompletions = getCompletionsForSim(simStack, lastSeg);

    // Prepend resolvedPart to each completion and requote if needed
    std::vector<std::string> result;
    result.reserve(rawCompletions.size());
    for (const auto& c : rawCompletions) {
        // Strip surrounding quotes from the individual completion name if present
        std::string raw = c;
        if (raw.size() >= 2 && raw.front() == '"' && raw.back() == '"') {
            raw = raw.substr(1, raw.size() - 2);
        }
        std::string full = resolvedPart + raw;
        if (full.find(' ') != std::string::npos) {
            result.push_back("\"" + full + "\"");
        } else {
            result.push_back(full);
        }
    }
    return result;
}

