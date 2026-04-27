#include "select.h"
#include "../exceptions/exceptions.h"

#include <iostream>
#include <string>
#include <cctype>
#include "../utils/utils.h"

namespace commands {

// Helper function to check if a type is a leaf node (cannot navigate further)
bool isLeafType(Path::ItemType type) {
    switch (type) {
        case Path::EMPLOYEE:
        case Path::PROJECT_TEAM_MEMBER:
        case Path::ACTIVITY_NOTE:
        case Path::HOURS:
        case Path::ARTEFACT_DATA_ITEM:
        case Path::MILESTONE_TO_ARTEFACT_LINK:
        case Path::ARTEFACT_FIELD:
        case Path::MILESTONE_STEP:
            return true;
        default:
            return false;
    }
}

// Helper function to resolve folder names to indices
int getFolderIndex(Path::ItemType currentType, const std::string& name) {
    std::string lowerName = utils::toLower(name);

    switch (currentType) {
    case Path::ROOT:
        if (lowerName == "organization") return 1;
        if (lowerName == "projects") return 2;
        break;
    case Path::ORGANIZATION_FOLDER:
        if (lowerName == "departments") return 1;
        break;
    case Path::PROJECT:
        if (lowerName == "activity categories" || lowerName == "activitycategories") return 1;
        if (lowerName == "artefact types" || lowerName == "artefacttypes") return 2;
        if (lowerName == "milestones") return 3;
        if (lowerName == "team") return 4;
        break;
    case Path::ACTIVITY:
        if (lowerName == "tasks") return 1;
        if (lowerName == "notes") return 2;
        break;
    case Path::TASK:
    case Path::SUBTASK:
        if (lowerName == "tasks" || lowerName == "subtasks") return 1;
        if (lowerName == "assignments") return 2;
        break;
    case Path::ARTEFACT_TYPE:
        if (lowerName == "artefacts") return 1;
        if (lowerName == "fields") return 2;
        break;
    case Path::ARTEFACT:
        if (lowerName == "data") return 1;
        if (lowerName == "associations") return 2;
        break;
    default:
        break;
    }
    return -1; // Invalid folder name
}

// Helper to navigate to a folder by index
void selectFolderByIndex(Path& pth, int id) {
    switch (pth.current_type()) {
    case Path::ROOT:
        switch (id) {
        case 1:
            pth.move_forward("Organization", id, Path::ORGANIZATION_FOLDER);
            break;
        case 2:
            pth.move_forward("Projects", id, Path::PROJECTS_FOLDER);
            break;
        default:
            throw NavigationError("Invalid selection.");
        }
        break;
    case Path::ORGANIZATION_FOLDER:
        switch (id) {
        case 1:
            pth.move_forward("Departments", id, Path::DEPARTMENTS_FOLDER);
            break;
        default:
            throw NavigationError("Invalid selection.");
        }
        break;
    case Path::PROJECT:
        switch (id) {
        case 1:
            pth.move_forward("Activity Categories", pth.current_id(), Path::ACTIVITIES_FOLDER);
            break;
        case 2:
            pth.move_forward("Artefact Types", pth.current_id(), Path::ARTEFACT_TYPES_FOLDER);
            break;
        case 3:
            pth.move_forward("Milestones", pth.current_id(), Path::MILESTONES_FOLDER);
            break;
        case 4:
            pth.move_forward("Team", pth.current_id(), Path::TEAM_FOLDER);
            break;
        default:
            throw NavigationError("Invalid selection.");
        }
        break;
    case Path::ACTIVITY:
        switch (id) {
        case 1:
            pth.move_forward("Tasks", pth.current_id(), Path::TASKS_FOLDER);
            break;
        case 2:
            pth.move_forward("Notes", pth.current_id(), Path::NOTES_FOLDER);
            break;
        default:
            throw NavigationError("Invalid selection.");
        }
        break;
    case Path::TASK:
    case Path::SUBTASK:
        switch (id) {
        case 1:
            pth.move_forward("Tasks", pth.current_id(), Path::TASKS_FOLDER);
            break;
        case 2:
            pth.move_forward("Assignments", pth.current_id(), Path::ASSIGNMENTS_FOLDER);
            break;
        default:
            throw NavigationError("Invalid selection.");
        }
        break;
    case Path::ARTEFACT_TYPE:
        switch (id) {
        case 1:
            pth.move_forward("Artefacts", pth.current_id(), Path::ARTEFACTS_FOLDER);
            break;
        case 2:
            pth.move_forward("Fields", pth.current_id(), Path::FIELDS_FOLDER);
            break;
        default:
            throw NavigationError("Invalid selection.");
        }
        break;
    case Path::ARTEFACT:
        switch (id) {
        case 1:
            pth.move_forward("Data", pth.current_id(), Path::DATA_FOLDER);
            break;
        case 2:
            pth.move_forward("Associations", pth.current_id(), Path::ASSOCIATIONS_FOLDER);
            break;
        default:
            throw NavigationError("Invalid selection.");
        }
        break;
    default:
        throw NavigationError("Invalid selection.");
        break;
    }
}

// Single-segment navigation (no path separators). Handles IDs, folder names, and entity names.
static void selectSingle(Database& db, Path& pth, const std::string& segment) {
    bool useId = std::isdigit(static_cast<unsigned char>(segment[0]));

    if (useId) {
        int id = utils::toInt(segment);
        switch (pth.current_type()) {
        case Path::ROOT:
        case Path::ORGANIZATION_FOLDER:
        case Path::PROJECT:
        case Path::ACTIVITY:
        case Path::TASK:
        case Path::SUBTASK:
        case Path::ARTEFACT_TYPE:
        case Path::ARTEFACT:
            selectFolderByIndex(pth, id);
            break;
        case Path::DEPARTMENTS_FOLDER:
            selectDepartment(db, pth, id);
            break;
        case Path::DEPARTMENT:
            selectEmployee(db, pth, id);
            break;
        case Path::PROJECTS_FOLDER:
            selectProject(db, pth, id);
            break;
        case Path::TEAM_FOLDER:
            selectTeamMember(db, pth, id);
            break;
        case Path::ACTIVITIES_FOLDER:
            selectCategory(db, pth, id);
            break;
        case Path::CATEGORY:
            selectSubCategory(db, pth, id);
            break;
        case Path::SUBCATEGORY:
            selectActivity(db, pth, id);
            break;
        case Path::TASKS_FOLDER:
            if (pth.getIdByType(Path::TASK) > 0) {
                selectSubTask(db, pth, id);
            } else {
                selectTask(db, pth, id);
            }
            break;
        case Path::ASSIGNMENTS_FOLDER:
            selectAssignment(db, pth, id);
            break;
        case Path::ASSIGNMENT:
            selectHours(db, pth, id);
            break;
        case Path::NOTES_FOLDER:
            selectNote(db, pth, id);
            break;
        case Path::ARTEFACT_TYPES_FOLDER:
            selectArtefactType(db, pth, id);
            break;
        case Path::ARTEFACTS_FOLDER:
            selectArtefact(db, pth, id);
            break;
        case Path::DATA_FOLDER:
            selectArtefactDataItem(db, pth, id);
            break;
        case Path::ASSOCIATIONS_FOLDER:
            selectArtefactToActivityLink(db, pth, id);
            break;
        case Path::ARTEFACT_TO_ACTIVITY_LINK:
            selectMilestoneToArtefactLink(db, pth, id);
            break;
        case Path::FIELDS_FOLDER:
            selectField(db, pth, id);
            break;
        case Path::MILESTONES_FOLDER:
            selectMilestone(db, pth, id);
            break;
        case Path::MILESTONE:
            selectMilestoneStep(db, pth, id);
            break;
        default:
            throw NavigationError("Cannot navigate further from this location.");
            break;
        }
    } else {
        // Name-based selection — check folder names first
        int folderIdx = getFolderIndex(pth.current_type(), segment);
        if (folderIdx > 0) {
            selectFolderByIndex(pth, folderIdx);
        } else {
            switch (pth.current_type()) {
            case Path::DEPARTMENTS_FOLDER:
                selectDepartment(db, pth, segment);
                break;
            case Path::DEPARTMENT:
                selectEmployee(db, pth, segment);
                break;
            case Path::PROJECTS_FOLDER:
                selectProject(db, pth, segment);
                break;
            case Path::ACTIVITIES_FOLDER:
                selectCategory(db, pth, segment);
                break;
            case Path::CATEGORY:
                selectSubCategory(db, pth, segment);
                break;
            case Path::SUBCATEGORY:
                selectActivity(db, pth, segment);
                break;
            case Path::TASKS_FOLDER:
                if (pth.getIdByType(Path::TASK) > 0) {
                    selectSubTask(db, pth, segment);
                } else {
                    selectTask(db, pth, segment);
                }
                break;
            case Path::TASK:
                selectSubTask(db, pth, segment);
                break;
            case Path::ASSIGNMENTS_FOLDER:
                selectAssignment(db, pth, segment);
                break;
            case Path::NOTES_FOLDER:
                selectNote(db, pth, segment);
                break;
            case Path::ARTEFACT_TYPES_FOLDER:
                selectArtefactType(db, pth, segment);
                break;
            case Path::ARTEFACTS_FOLDER:
                selectArtefact(db, pth, segment);
                break;
            case Path::FIELDS_FOLDER:
                selectField(db, pth, segment);
                break;
            case Path::MILESTONES_FOLDER:
                selectMilestone(db, pth, segment);
                break;
            case Path::MILESTONE:
                selectMilestoneStep(db, pth, segment);
                break;
            case Path::ASSOCIATIONS_FOLDER:
                selectArtefactToActivityLink(db, pth, segment);
                break;
            default:
                throw NavigationError("Invalid selection '" + segment + "'.");
                break;
            }
        }
    }
}

void select(Database& db, Path& pth, std::string param) {
    if (param.find('/') == std::string::npos) {
        // Single-segment behavior (unchanged)
        if (param == "..") {
            pth.move_back();
        } else if (param == "~") {
            pth.reset();
        } else if (!param.empty()) {
            selectSingle(db, pth, param);
        }
        return;
    }

    // Path-based navigation
    std::string pathStr = param;
    if (!pathStr.empty() && pathStr[0] == '~') {
        pth.reset();
        pathStr = pathStr.substr(1);
        if (!pathStr.empty() && pathStr[0] == '/') {
            pathStr = pathStr.substr(1);
        }
    }

    auto segments = utils::split(pathStr, '/');
    for (const auto& seg : segments) {
        if (seg.empty() || seg == ".") {
            continue;
        } else if (seg == "..") {
            pth.move_back();
        } else {
            selectSingle(db, pth, seg);
        }
    }
}

void selectProject(Database& db, Path& pth, int id) {
    std::string sql;
    sql = "SELECT projectname FROM Projects WHERE projectid = ?";
    auto stmt = db.prepareStatement(sql);
    stmt->setInt(1,id);
    auto res = stmt->executeQuery();
    std::string name;
    if (res->next()){
        name = res->getString("projectname");
        pth.move_forward(name,id);
    }else{
        throw EntityNotFoundError("Project", id);
    }
}

void selectCategory(Database& db, Path& pth, int id) {
    std::string sql;
    sql = "SELECT categoryname FROM ActivityCategories WHERE categoryid = ? AND projectid = ?";
    auto stmt = db.prepareStatement(sql);
    stmt->setInt(1,id);
    stmt->setInt(2,pth.current_id());
    auto res = stmt->executeQuery();
    std::string name;
    if (res->next()){
        name = res->getString("categoryname");
        pth.move_forward(name,id);
    }else{
        throw EntityNotFoundError("Category", id);
    }
}

void selectSubCategory(Database& db, Path& pth, int id) {
    std::string sql;
    sql = "SELECT subcategoryname FROM ActivitySubcategories WHERE subcategoryid = ? AND categoryid = ?";
    auto stmt = db.prepareStatement(sql);
    stmt->setInt(1,id);
    stmt->setInt(2,pth.current_id());
    auto res = stmt->executeQuery();
    std::string name;
    if (res->next()){
        name = res->getString("subcategoryname");
        pth.move_forward(name,id);
    }else{
        throw EntityNotFoundError("Sub-category", id);
    }
}

void selectActivity(Database& db, Path& pth, int id) {
    std::string sql;
    sql = "SELECT activityname FROM Activities WHERE activityid = ? AND subcategoryid = ?";
    auto stmt = db.prepareStatement(sql);
    stmt->setInt(1,id);
    stmt->setInt(2,pth.current_id());
    auto res = stmt->executeQuery();
    std::string name;
    if (res->next()){
        name = res->getString("activityname");
        pth.move_forward(name,id);
    }else{
        throw EntityNotFoundError("Activity", id);
    }
}

void selectTask(Database& db, Path& pth, int id) {
    std::string sql;
    sql = "SELECT taskname FROM ActivityTasks WHERE taskid = ? AND activityid = ?";
    auto stmt = db.prepareStatement(sql);
    stmt->setInt(1,id);
    stmt->setInt(2,pth.current_id());
    auto res = stmt->executeQuery();
    std::string name;
    if (res->next()){
        name = res->getString("taskname");
        pth.move_forward(name,id);
    }else{
        throw EntityNotFoundError("Task", id);
    }
}

void selectSubTask(Database& db, Path& pth, int id) {
    std::string sql;
    sql = "SELECT taskname FROM ActivityTasks WHERE taskid = ? AND parenttaskid = ?";
    auto stmt = db.prepareStatement(sql);
    stmt->setInt(1,id);
    stmt->setInt(2,pth.current_id());
    auto res = stmt->executeQuery();
    std::string name;
    if (res->next()){
        name = res->getString("taskname");
        pth.move_forward(name,id);
    }else{
        throw EntityNotFoundError("Sub-task", id);
    }
}

void selectEmployee(Database& db, Path& pth, int id) {
    std::string sql = "SELECT employeeid, firstname, lastname, email, departmentid FROM Employees WHERE employeeid = ?";
    auto stmt = db.prepareStatement(sql);
    stmt->setInt(1, id);
    auto res = stmt->executeQuery();

    if (res->next()) {
        // Display employee details instead of navigating (leaf node)
        std::cout << "Employee Details:" << std::endl;
        std::cout << "  ID: " << res->getInt("employeeid") << std::endl;
        std::cout << "  Name: " << res->getString("firstname") << " " << res->getString("lastname") << std::endl;
        std::cout << "  Email: " << res->getString("email") << std::endl;
        if (!res->isNull("departmentid")) {
            std::cout << "  Department ID: " << res->getInt("departmentid") << std::endl;
        }
    } else {
        throw EntityNotFoundError("Employee", id);
    }
}

void selectNote(Database& db, Path& pth, int id) {
    std::string sql = "SELECT an.noteid, an.note, an.notedate, CONCAT(e.firstname, ' ', e.lastname) as username "
                      "FROM ActivityNotes an JOIN Employees e ON an.userid = e.employeeid "
                      "WHERE an.noteid = ? AND an.activityid = ?";
    auto stmt = db.prepareStatement(sql);
    stmt->setInt(1, id);
    stmt->setInt(2, pth.current_id());
    auto res = stmt->executeQuery();

    if (res->next()) {
        // Display note details instead of navigating (leaf node)
        std::cout << "Note Details:" << std::endl;
        std::cout << "  ID: " << res->getInt("noteid") << std::endl;
        std::cout << "  Date: " << res->getString("notedate") << std::endl;
        std::cout << "  By: " << res->getString("username") << std::endl;
        std::cout << "  Note: " << res->getString("note") << std::endl;
    } else {
        throw EntityNotFoundError("Note", id);
    }
}

void selectArtefactType (Database& db, Path& pth, int id) {
    std::string sql = "SELECT artefactname FROM ArtefactTypes WHERE artefacttypeid = ? AND projectid = ?";
    auto stmt = db.prepareStatement(sql);
    stmt->setInt(1, id);
    stmt->setInt(2, pth.current_id());
    auto res = stmt->executeQuery();

    if (res->next()) {
        std::string name = res->getString("artefactname");
        pth.move_forward(name, id, Path::ARTEFACT_TYPE);
    } else {
        throw EntityNotFoundError("Artefact type", id);
    }
}

void selectArtefact (Database& db, Path& pth, int id) {
    std::string sql = "SELECT artefacttitle FROM Artefacts WHERE artefactid = ? AND artefacttypeid = ?";
    auto stmt = db.prepareStatement(sql);
    stmt->setInt(1, id);
    stmt->setInt(2, pth.current_id());
    auto res = stmt->executeQuery();

    if (res->next()) {
        std::string name = res->getString("artefacttitle");
        pth.move_forward(name, id, Path::ARTEFACT);
    } else {
        throw EntityNotFoundError("Artefact", id);
    }
}

void selectField (Database& db, Path& pth, int id) {
    std::string sql = "SELECT artefactdatafieldid, fieldtitle, valuetype, maximumlength, maximumvalue, minimumvalue "
                      "FROM ArtefactDataFields WHERE artefactdatafieldid = ? AND artefacttypeid = ?";
    auto stmt = db.prepareStatement(sql);
    stmt->setInt(1, id);
    stmt->setInt(2, pth.current_id());
    auto res = stmt->executeQuery();

    if (res->next()) {
        // Display field details instead of navigating (leaf node)
        std::cout << "Field Details:" << std::endl;
        std::cout << "  ID: " << res->getInt("artefactdatafieldid") << std::endl;
        std::cout << "  Title: " << res->getString("fieldtitle") << std::endl;
        std::cout << "  Type: " << (res->getInt("valuetype") == 0 ? "Text" : "Number") << std::endl;
        if (res->getInt("valuetype") == 0 && !res->isNull("maximumlength")) {
            std::cout << "  Max Length: " << res->getInt("maximumlength") << std::endl;
        }
        if (res->getInt("valuetype") == 1) {
            if (!res->isNull("minimumvalue")) {
                std::cout << "  Min Value: " << res->getInt("minimumvalue") << std::endl;
            }
            if (!res->isNull("maximumvalue")) {
                std::cout << "  Max Value: " << res->getInt("maximumvalue") << std::endl;
            }
        }
    } else {
        throw EntityNotFoundError("Field", id);
    }
}

void selectMilestone (Database& db, Path& pth, int id) {
    std::string sql = "SELECT milestonename FROM Milestones WHERE milestoneid = ? AND projectid = ?";
    auto stmt = db.prepareStatement(sql);
    stmt->setInt(1, id);
    stmt->setInt(2, pth.current_id());
    auto res = stmt->executeQuery();

    if (res->next()) {
        std::string name = res->getString("milestonename");
        pth.move_forward(name, id, Path::MILESTONE);
    } else {
        throw EntityNotFoundError("Milestone", id);
    }
}

void selectMilestoneStep (Database& db, Path& pth, int id) {
    std::string sql = "SELECT milestonestepid, milestonestepname, progressratio FROM MilestoneSteps WHERE milestonestepid = ? AND milestoneid = ?";
    auto stmt = db.prepareStatement(sql);
    stmt->setInt(1, id);
    stmt->setInt(2, pth.current_id());
    auto res = stmt->executeQuery();

    if (res->next()) {
        // Display milestone step details instead of navigating (leaf node)
        std::cout << "Milestone Step Details:" << std::endl;
        std::cout << "  ID: " << res->getInt("milestonestepid") << std::endl;
        std::cout << "  Name: " << res->getString("milestonestepname") << std::endl;
        std::cout << "  Progress Ratio: " << res->getDouble("progressratio") << std::endl;
    } else {
        throw EntityNotFoundError("Milestone step", id);
    }
}

// ============================================================================
// Name-based selector overloads
// ============================================================================

void selectProject(Database& db, Path& pth, const std::string& name) {
    std::string sql = "SELECT projectid, projectname FROM Projects WHERE LOWER(projectname) = LOWER(?) LIMIT 2";
    auto stmt = db.prepareStatement(sql);
    stmt->setString(1, name);
    auto res = stmt->executeQuery();

    if (res->next()) {
        int id = res->getInt("projectid");
        std::string actualName = res->getString("projectname");
        if (res->next()) {
            std::cerr << "Warning: Multiple projects found with name '" << name
                      << "'. Selecting first match (ID: " << id << ")." << std::endl;
        }
        pth.move_forward(actualName, id);
    } else {
        throw EntityNotFoundError("Project", name);
    }
}

void selectCategory(Database& db, Path& pth, const std::string& name) {
    std::string sql = "SELECT categoryid, categoryname FROM ActivityCategories WHERE LOWER(categoryname) = LOWER(?) AND projectid = ? LIMIT 2";
    auto stmt = db.prepareStatement(sql);
    stmt->setString(1, name);
    stmt->setInt(2, pth.current_id());
    auto res = stmt->executeQuery();

    if (res->next()) {
        int id = res->getInt("categoryid");
        std::string actualName = res->getString("categoryname");
        if (res->next()) {
            std::cerr << "Warning: Multiple categories found with name '" << name
                      << "'. Selecting first match (ID: " << id << ")." << std::endl;
        }
        pth.move_forward(actualName, id);
    } else {
        throw EntityNotFoundError("Category", name);
    }
}

void selectSubCategory(Database& db, Path& pth, const std::string& name) {
    std::string sql = "SELECT subcategoryid, subcategoryname FROM ActivitySubcategories WHERE LOWER(subcategoryname) = LOWER(?) AND categoryid = ? LIMIT 2";
    auto stmt = db.prepareStatement(sql);
    stmt->setString(1, name);
    stmt->setInt(2, pth.current_id());
    auto res = stmt->executeQuery();

    if (res->next()) {
        int id = res->getInt("subcategoryid");
        std::string actualName = res->getString("subcategoryname");
        if (res->next()) {
            std::cerr << "Warning: Multiple sub-categories found with name '" << name
                      << "'. Selecting first match (ID: " << id << ")." << std::endl;
        }
        pth.move_forward(actualName, id);
    } else {
        throw EntityNotFoundError("Sub-category", name);
    }
}

void selectActivity(Database& db, Path& pth, const std::string& name) {
    std::string sql = "SELECT activityid, activityname FROM Activities WHERE LOWER(activityname) = LOWER(?) AND subcategoryid = ? LIMIT 2";
    auto stmt = db.prepareStatement(sql);
    stmt->setString(1, name);
    stmt->setInt(2, pth.current_id());
    auto res = stmt->executeQuery();

    if (res->next()) {
        int id = res->getInt("activityid");
        std::string actualName = res->getString("activityname");
        if (res->next()) {
            std::cerr << "Warning: Multiple activities found with name '" << name
                      << "'. Selecting first match (ID: " << id << ")." << std::endl;
        }
        pth.move_forward(actualName, id);
    } else {
        throw EntityNotFoundError("Activity", name);
    }
}

void selectTask(Database& db, Path& pth, const std::string& name) {
    std::string sql = "SELECT taskid, taskname FROM ActivityTasks WHERE LOWER(taskname) = LOWER(?) AND activityid = ? AND parenttaskid IS NULL LIMIT 2";
    auto stmt = db.prepareStatement(sql);
    stmt->setString(1, name);
    stmt->setInt(2, pth.current_id());
    auto res = stmt->executeQuery();

    if (res->next()) {
        int id = res->getInt("taskid");
        std::string actualName = res->getString("taskname");
        if (res->next()) {
            std::cerr << "Warning: Multiple tasks found with name '" << name
                      << "'. Selecting first match (ID: " << id << ")." << std::endl;
        }
        pth.move_forward(actualName, id);
    } else {
        throw EntityNotFoundError("Task", name);
    }
}

void selectSubTask(Database& db, Path& pth, const std::string& name) {
    std::string sql = "SELECT taskid, taskname FROM ActivityTasks WHERE LOWER(taskname) = LOWER(?) AND parenttaskid = ? LIMIT 2";
    auto stmt = db.prepareStatement(sql);
    stmt->setString(1, name);
    stmt->setInt(2, pth.current_id());
    auto res = stmt->executeQuery();

    if (res->next()) {
        int id = res->getInt("taskid");
        std::string actualName = res->getString("taskname");
        if (res->next()) {
            std::cerr << "Warning: Multiple sub-tasks found with name '" << name
                      << "'. Selecting first match (ID: " << id << ")." << std::endl;
        }
        pth.move_forward(actualName, id);
    } else {
        throw EntityNotFoundError("Sub-task", name);
    }
}

void selectEmployee(Database& db, Path& pth, const std::string& name) {
    // Match against full name (firstname + lastname), or just firstname, or just lastname
    std::string sql = R"(
        SELECT employeeid, firstname, lastname, email, departmentid
        FROM Employees
        WHERE LOWER(CONCAT(firstname, ' ', lastname)) = LOWER(?)
           OR LOWER(firstname) = LOWER(?)
           OR LOWER(lastname) = LOWER(?)
        LIMIT 2
    )";
    auto stmt = db.prepareStatement(sql);
    stmt->setString(1, name);
    stmt->setString(2, name);
    stmt->setString(3, name);
    auto res = stmt->executeQuery();

    if (res->next()) {
        int id = res->getInt("employeeid");
        if (res->next()) {
            std::cerr << "Warning: Multiple employees found with name '" << name
                      << "'. Showing first match (ID: " << id << ")." << std::endl;
        }
        // Go back to first result
        res->first();
        // Display employee details instead of navigating (leaf node)
        std::cout << "Employee Details:" << std::endl;
        std::cout << "  ID: " << res->getInt("employeeid") << std::endl;
        std::cout << "  Name: " << res->getString("firstname") << " " << res->getString("lastname") << std::endl;
        std::cout << "  Email: " << res->getString("email") << std::endl;
        if (!res->isNull("departmentid")) {
            std::cout << "  Department ID: " << res->getInt("departmentid") << std::endl;
        }
    } else {
        throw EntityNotFoundError("Employee", name);
    }
}

void selectNote(Database& db, Path& pth, const std::string& name) {
    // Try to match by date first (YYYY-MM-DD format), otherwise match partial note text
    std::string sql;
    std::unique_ptr<sql::PreparedStatement> stmt;

    if (utils::isValidDate(name)) {
        sql = "SELECT an.noteid, an.note, an.notedate, CONCAT(e.firstname, ' ', e.lastname) as username "
              "FROM ActivityNotes an JOIN Employees e ON an.userid = e.employeeid "
              "WHERE an.notedate = ? AND an.activityid = ? LIMIT 2";
        stmt = db.prepareStatement(sql);
        stmt->setString(1, name);
        stmt->setInt(2, pth.current_id());
    } else {
        sql = "SELECT an.noteid, an.note, an.notedate, CONCAT(e.firstname, ' ', e.lastname) as username "
              "FROM ActivityNotes an JOIN Employees e ON an.userid = e.employeeid "
              "WHERE LOWER(an.note) LIKE LOWER(?) AND an.activityid = ? LIMIT 2";
        stmt = db.prepareStatement(sql);
        stmt->setString(1, "%" + name + "%");
        stmt->setInt(2, pth.current_id());
    }

    auto res = stmt->executeQuery();

    if (res->next()) {
        int id = res->getInt("noteid");
        if (res->next()) {
            std::cerr << "Warning: Multiple notes found matching '" << name
                      << "'. Showing first match (ID: " << id << ")." << std::endl;
        }
        // Go back to first result
        res->first();
        // Display note details instead of navigating (leaf node)
        std::cout << "Note Details:" << std::endl;
        std::cout << "  ID: " << res->getInt("noteid") << std::endl;
        std::cout << "  Date: " << res->getString("notedate") << std::endl;
        std::cout << "  By: " << res->getString("username") << std::endl;
        std::cout << "  Note: " << res->getString("note") << std::endl;
    } else {
        throw EntityNotFoundError("Note", name);
    }
}

void selectArtefactType(Database& db, Path& pth, const std::string& name) {
    std::string sql = "SELECT artefacttypeid, artefactname FROM ArtefactTypes WHERE LOWER(artefactname) = LOWER(?) AND projectid = ? LIMIT 2";
    auto stmt = db.prepareStatement(sql);
    stmt->setString(1, name);
    stmt->setInt(2, pth.current_id());
    auto res = stmt->executeQuery();

    if (res->next()) {
        int id = res->getInt("artefacttypeid");
        std::string actualName = res->getString("artefactname");
        if (res->next()) {
            std::cerr << "Warning: Multiple artefact types found with name '" << name
                      << "'. Selecting first match (ID: " << id << ")." << std::endl;
        }
        pth.move_forward(actualName, id, Path::ARTEFACT_TYPE);
    } else {
        throw EntityNotFoundError("Artefact type", name);
    }
}

void selectArtefact(Database& db, Path& pth, const std::string& name) {
    std::string sql = "SELECT artefactid, artefacttitle FROM Artefacts WHERE LOWER(artefacttitle) = LOWER(?) AND artefacttypeid = ? LIMIT 2";
    auto stmt = db.prepareStatement(sql);
    stmt->setString(1, name);
    stmt->setInt(2, pth.current_id());
    auto res = stmt->executeQuery();

    if (res->next()) {
        int id = res->getInt("artefactid");
        std::string actualName = res->getString("artefacttitle");
        if (res->next()) {
            std::cerr << "Warning: Multiple artefacts found with name '" << name
                      << "'. Selecting first match (ID: " << id << ")." << std::endl;
        }
        pth.move_forward(actualName, id, Path::ARTEFACT);
    } else {
        throw EntityNotFoundError("Artefact", name);
    }
}

void selectField(Database& db, Path& pth, const std::string& name) {
    std::string sql = "SELECT artefactdatafieldid, fieldtitle, valuetype, maximumlength, maximumvalue, minimumvalue "
                      "FROM ArtefactDataFields WHERE LOWER(fieldtitle) = LOWER(?) AND artefacttypeid = ? LIMIT 2";
    auto stmt = db.prepareStatement(sql);
    stmt->setString(1, name);
    stmt->setInt(2, pth.current_id());
    auto res = stmt->executeQuery();

    if (res->next()) {
        int id = res->getInt("artefactdatafieldid");
        if (res->next()) {
            std::cerr << "Warning: Multiple fields found with name '" << name
                      << "'. Showing first match (ID: " << id << ")." << std::endl;
        }
        // Go back to first result
        res->first();
        // Display field details instead of navigating (leaf node)
        std::cout << "Field Details:" << std::endl;
        std::cout << "  ID: " << res->getInt("artefactdatafieldid") << std::endl;
        std::cout << "  Title: " << res->getString("fieldtitle") << std::endl;
        std::cout << "  Type: " << (res->getInt("valuetype") == 0 ? "Text" : "Number") << std::endl;
        if (res->getInt("valuetype") == 0 && !res->isNull("maximumlength")) {
            std::cout << "  Max Length: " << res->getInt("maximumlength") << std::endl;
        }
        if (res->getInt("valuetype") == 1) {
            if (!res->isNull("minimumvalue")) {
                std::cout << "  Min Value: " << res->getInt("minimumvalue") << std::endl;
            }
            if (!res->isNull("maximumvalue")) {
                std::cout << "  Max Value: " << res->getInt("maximumvalue") << std::endl;
            }
        }
    } else {
        throw EntityNotFoundError("Field", name);
    }
}

void selectMilestone(Database& db, Path& pth, const std::string& name) {
    std::string sql = "SELECT milestoneid, milestonename FROM Milestones WHERE LOWER(milestonename) = LOWER(?) AND projectid = ? LIMIT 2";
    auto stmt = db.prepareStatement(sql);
    stmt->setString(1, name);
    stmt->setInt(2, pth.current_id());
    auto res = stmt->executeQuery();

    if (res->next()) {
        int id = res->getInt("milestoneid");
        std::string actualName = res->getString("milestonename");
        if (res->next()) {
            std::cerr << "Warning: Multiple milestones found with name '" << name
                      << "'. Selecting first match (ID: " << id << ")." << std::endl;
        }
        pth.move_forward(actualName, id, Path::MILESTONE);
    } else {
        throw EntityNotFoundError("Milestone", name);
    }
}

void selectMilestoneStep(Database& db, Path& pth, const std::string& name) {
    std::string sql = "SELECT milestonestepid, milestonestepname, progressratio FROM MilestoneSteps WHERE LOWER(milestonestepname) = LOWER(?) AND milestoneid = ? LIMIT 2";
    auto stmt = db.prepareStatement(sql);
    stmt->setString(1, name);
    stmt->setInt(2, pth.current_id());
    auto res = stmt->executeQuery();

    if (res->next()) {
        int id = res->getInt("milestonestepid");
        if (res->next()) {
            std::cerr << "Warning: Multiple milestone steps found with name '" << name
                      << "'. Showing first match (ID: " << id << ")." << std::endl;
        }
        // Go back to first result
        res->first();
        // Display milestone step details instead of navigating (leaf node)
        std::cout << "Milestone Step Details:" << std::endl;
        std::cout << "  ID: " << res->getInt("milestonestepid") << std::endl;
        std::cout << "  Name: " << res->getString("milestonestepname") << std::endl;
        std::cout << "  Progress Ratio: " << res->getDouble("progressratio") << std::endl;
    } else {
        throw EntityNotFoundError("Milestone step", name);
    }
}

void selectDepartment(Database& db, Path& pth, int id) {
    try {
        auto stmt = db.prepareStatement(
            "SELECT departmentid, departmentname FROM Departments WHERE departmentid = ?"
        );
        stmt->setInt(1, id);
        auto res = stmt->executeQuery();

        if (res->next()) {
            std::string name = res->getString("departmentname");
            pth.move_forward(name, id, Path::DEPARTMENT);
        } else {
            throw EntityNotFoundError("Department", id);
        }
    } catch (sql::SQLException& e) {
        throw DatabaseError("selecting department", e.what());
    }
}

void selectDepartment(Database& db, Path& pth, const std::string& name) {
    try {
        auto stmt = db.prepareStatement(
            "SELECT departmentid, departmentname FROM Departments WHERE LOWER(departmentname) = LOWER(?)"
        );
        stmt->setString(1, name);
        auto res = stmt->executeQuery();

        if (res->next()) {
            int id = res->getInt("departmentid");
            std::string deptName = res->getString("departmentname");
            pth.move_forward(deptName, id, Path::DEPARTMENT);
        } else {
            throw EntityNotFoundError("Department", name);
        }
    } catch (sql::SQLException& e) {
        throw DatabaseError("selecting department", e.what());
    }
}

void selectTeamMember(Database& db, Path& pth, int id) {
    try {
        auto stmt = db.prepareStatement(
            "SELECT ptm.teammemberid, CONCAT(e.firstname, ' ', e.lastname) AS name, ptm.role "
            "FROM ProjectTeamMembers ptm "
            "JOIN Employees e ON ptm.employeeid = e.employeeid "
            "WHERE ptm.teammemberid = ? AND ptm.projectid = ?"
        );
        stmt->setInt(1, id);
        stmt->setInt(2, pth.getIdByType(Path::PROJECT));
        auto res = stmt->executeQuery();

        if (res->next()) {
            // Display team member details instead of navigating (leaf node)
            std::cout << "Team Member Details:" << std::endl;
            std::cout << "  ID: " << res->getInt("teammemberid") << std::endl;
            std::cout << "  Name: " << res->getString("name") << std::endl;
            int role = res->getInt("role");
            std::string roleStr;
            switch (role) {
                case 1: roleStr = "Team Member"; break;
                case 2: roleStr = "Project Manager"; break;
                case 3: roleStr = "Administrator"; break;
                default: roleStr = "Unknown"; break;
            }
            std::cout << "  Role: " << roleStr << std::endl;
        } else {
            throw EntityNotFoundError("Team member", id);
        }
    } catch (sql::SQLException& e) {
        throw DatabaseError("selecting team member", e.what());
    }
}

void selectAssignment(Database& db, Path& pth, int id) {
    try {
        auto stmt = db.prepareStatement(
            "SELECT ata.assignmentid, CONCAT(e.firstname, ' ', e.lastname) AS name "
            "FROM ActivityTaskAssignments ata "
            "JOIN Employees e ON ata.userid = e.employeeid "
            "WHERE ata.assignmentid = ? AND ata.taskid = ?"
        );
        stmt->setInt(1, id);
        stmt->setInt(2, pth.current_id());
        auto res = stmt->executeQuery();

        if (res->next()) {
            std::string name = res->getString("name");
            pth.move_forward(name, id, Path::ASSIGNMENT);
        } else {
            throw EntityNotFoundError("Assignment", id);
        }
    } catch (sql::SQLException& e) {
        throw DatabaseError("selecting assignment", e.what());
    }
}

void selectAssignment(Database& db, Path& pth, const std::string& name) {
    try {
        auto stmt = db.prepareStatement(
            "SELECT ata.assignmentid, CONCAT(e.firstname, ' ', e.lastname) AS name "
            "FROM ActivityTaskAssignments ata "
            "JOIN Employees e ON ata.userid = e.employeeid "
            "WHERE (LOWER(CONCAT(e.firstname, ' ', e.lastname)) = LOWER(?) "
            "   OR LOWER(e.firstname) = LOWER(?) "
            "   OR LOWER(e.lastname) = LOWER(?)) "
            "AND ata.taskid = ? LIMIT 2"
        );
        stmt->setString(1, name);
        stmt->setString(2, name);
        stmt->setString(3, name);
        stmt->setInt(4, pth.current_id());
        auto res = stmt->executeQuery();

        if (res->next()) {
            int id = res->getInt("assignmentid");
            std::string actualName = res->getString("name");
            if (res->next()) {
                std::cerr << "Warning: Multiple assignments found for '" << name
                          << "'. Selecting first match (ID: " << id << ")." << std::endl;
            }
            pth.move_forward(actualName, id, Path::ASSIGNMENT);
        } else {
            throw EntityNotFoundError("Assignment", name);
        }
    } catch (sql::SQLException& e) {
        throw DatabaseError("selecting assignment", e.what());
    }
}

void selectHours(Database& db, Path& pth, int id) {
    try {
        auto stmt = db.prepareStatement(
            "SELECT bookingid, bookeddate, hours FROM Hours WHERE bookingid = ? AND assignmentid = ?"
        );
        stmt->setInt(1, id);
        stmt->setInt(2, pth.current_id());
        auto res = stmt->executeQuery();

        if (res->next()) {
            // Display hours details instead of navigating (leaf node)
            std::cout << "Hours Booking Details:" << std::endl;
            std::cout << "  ID: " << res->getInt("bookingid") << std::endl;
            std::cout << "  Date: " << res->getString("bookeddate") << std::endl;
            std::cout << "  Hours: " << res->getInt("hours") << std::endl;
        } else {
            throw EntityNotFoundError("Hours booking", id);
        }
    } catch (sql::SQLException& e) {
        throw DatabaseError("selecting hours", e.what());
    }
}

void selectArtefactDataItem(Database& db, Path& pth, int id) {
    try {
        auto stmt = db.prepareStatement(
            "SELECT ad.artefactdataid, adf.fieldtitle, ad.value "
            "FROM ArtefactData ad "
            "JOIN ArtefactDataFields adf ON ad.artefactdatafieldid = adf.artefactdatafieldid "
            "WHERE ad.artefactdataid = ? AND ad.artefactid = ?"
        );
        stmt->setInt(1, id);
        stmt->setInt(2, pth.getIdByType(Path::ARTEFACT));
        auto res = stmt->executeQuery();

        if (res->next()) {
            // Display artefact data item details instead of navigating (leaf node)
            std::cout << "Artefact Data Details:" << std::endl;
            std::cout << "  ID: " << res->getInt("artefactdataid") << std::endl;
            std::cout << "  Field: " << res->getString("fieldtitle") << std::endl;
            std::cout << "  Value: " << (res->isNull("value") ? "(null)" : res->getString("value")) << std::endl;
        } else {
            throw EntityNotFoundError("Artefact data item", id);
        }
    } catch (sql::SQLException& e) {
        throw DatabaseError("selecting artefact data item", e.what());
    }
}

void selectArtefactToActivityLink(Database& db, Path& pth, int id) {
    try {
        auto stmt = db.prepareStatement(
            "SELECT aal.artefactlinkid, a.activityname "
            "FROM ArtefactToActivityLink aal "
            "JOIN Activities a ON aal.activityid = a.activityid "
            "WHERE aal.artefactlinkid = ? AND aal.artefactid = ?"
        );
        stmt->setInt(1, id);
        stmt->setInt(2, pth.getIdByType(Path::ARTEFACT));
        auto res = stmt->executeQuery();

        if (res->next()) {
            std::string name = res->getString("activityname");
            pth.move_forward(name, id, Path::ARTEFACT_TO_ACTIVITY_LINK);
        } else {
            throw EntityNotFoundError("Artefact-activity link", id);
        }
    } catch (sql::SQLException& e) {
        throw DatabaseError("selecting artefact-activity link", e.what());
    }
}

void selectArtefactToActivityLink(Database& db, Path& pth, const std::string& name) {
    try {
        auto stmt = db.prepareStatement(
            "SELECT aal.artefactlinkid, a.activityname "
            "FROM ArtefactToActivityLink aal "
            "JOIN Activities a ON aal.activityid = a.activityid "
            "WHERE LOWER(a.activityname) = LOWER(?) AND aal.artefactid = ? LIMIT 2"
        );
        stmt->setString(1, name);
        stmt->setInt(2, pth.getIdByType(Path::ARTEFACT));
        auto res = stmt->executeQuery();

        if (res->next()) {
            int id = res->getInt("artefactlinkid");
            std::string actualName = res->getString("activityname");
            if (res->next()) {
                std::cerr << "Warning: Multiple artefact-activity links found for '" << name
                          << "'. Selecting first match (ID: " << id << ")." << std::endl;
            }
            pth.move_forward(actualName, id, Path::ARTEFACT_TO_ACTIVITY_LINK);
        } else {
            throw EntityNotFoundError("Artefact-activity link", name);
        }
    } catch (sql::SQLException& e) {
        throw DatabaseError("selecting artefact-activity link", e.what());
    }
}

void selectMilestoneToArtefactLink(Database& db, Path& pth, int id) {
    try {
        auto stmt = db.prepareStatement(
            "SELECT mal.milestonelinkid, ms.milestonestepname, mal.completiondate, "
            "CONCAT(e.firstname, ' ', e.lastname) AS completedby "
            "FROM MilestoneToArtefactLink mal "
            "JOIN MilestoneSteps ms ON mal.milestonestepid = ms.milestonestepid "
            "LEFT JOIN Employees e ON mal.completedbyid = e.employeeid "
            "WHERE mal.milestonelinkid = ? AND mal.artefactlinkid = ?"
        );
        stmt->setInt(1, id);
        stmt->setInt(2, pth.current_id());
        auto res = stmt->executeQuery();

        if (res->next()) {
            // Display milestone-artefact link details instead of navigating (leaf node)
            std::cout << "Milestone-Artefact Link Details:" << std::endl;
            std::cout << "  ID: " << res->getInt("milestonelinkid") << std::endl;
            std::cout << "  Milestone Step: " << res->getString("milestonestepname") << std::endl;
            if (!res->isNull("completiondate")) {
                std::cout << "  Completion Date: " << res->getString("completiondate") << std::endl;
            }
            if (!res->isNull("completedby")) {
                std::cout << "  Completed By: " << res->getString("completedby") << std::endl;
            }
        } else {
            throw EntityNotFoundError("Milestone-artefact link", id);
        }
    } catch (sql::SQLException& e) {
        throw DatabaseError("selecting milestone-artefact link", e.what());
    }
}

} // namespace commands
