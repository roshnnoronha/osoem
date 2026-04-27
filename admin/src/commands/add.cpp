#include "add.h"
#include "../exceptions/exceptions.h"
#include "../utils/auth.h"
#include "../utils/csv_parser.h"
#include <vector>

namespace commands {

void add(Database& db, Path& pth) {

  try {
    int parentId = pth.current_id(); 
    switch (pth.current_type()) {
        case Path::ROOT:
            throw NavigationError("Cannot add an item here.");
        case Path::ORGANIZATION_FOLDER:
            throw NavigationError("Cannot add an item here.");
        case Path::DEPARTMENTS_FOLDER :{
            DepartmentData dept(db);
            addDepartment(db, dept);
            break;
        }
        case Path::DEPARTMENT: {
            EmployeeData emp(db, parentId);
            addEmployee(db, emp);
            break;
        }
        case Path::PROJECTS_FOLDER: {
            ProjectData proj(db);
            addProject(db, proj);
            break;
        }
        case Path::PROJECT:
            throw NavigationError("Cannot add an item here.");
        case Path::ACTIVITIES_FOLDER: {
            CategoryData cat(db, parentId);
            addCategory(db, cat, parentId);
            break;
        }
        case Path::CATEGORY: {
            SubCategoryData subCat(db, parentId);
            addSubcategory(db, subCat, parentId);
            break;
        }
        case Path::SUBCATEGORY:{
            ActivityData act(db, parentId, pth.getIdByType(Path::PROJECT));
            addActivity(db, act, parentId, pth.getIdByType(Path::PROJECT));
            break;
        }
        case Path::ACTIVITY:
            throw NavigationError("Cannot add an item here.");
        case Path::TASKS_FOLDER:{
            // Check if we're in a sub-tasks folder (under a task) or main tasks folder (under an activity)
            if (pth.getIdByType(Path::TASK) > 0) {
                // We're in a sub-tasks folder - get the activity ID for this task
                TaskData tsk(db, pth.getIdByType(Path::ACTIVITY), parentId);
                addTask(db, tsk, pth.getIdByType(Path::ACTIVITY), parentId);
            }else {
                // We're in the main tasks folder under an activity
                TaskData tsk(db,parentId,0);
                addTask(db, tsk, parentId,0);
            }
            break;
        }
        case Path::TASK:
            throw NavigationError("Cannot add an item here. Navigate to Tasks folder first.");
        case Path::NOTES_FOLDER :{
            NoteData noe(db, parentId, pth.getIdByType(Path::PROJECT));
            addNote(db, noe, pth.getIdByType(Path::PROJECT));
            break;
        }
        case Path::ARTEFACT_TYPES_FOLDER :{
            ArtefactTypeData artTyp(db, parentId);
            addArtefactType(db, artTyp);
            break;
        }
        case Path::ARTEFACT_TYPE:
            throw NavigationError("Cannot add an item here.");
        case Path::ARTEFACTS_FOLDER:{
            ArtefactData art(db, parentId, pth.getIdByType(Path::PROJECT));
            addArtefact(db, art, pth.getIdByType(Path::PROJECT));
            break;
        }
        case Path::FIELDS_FOLDER:{
            FieldData fie(db, parentId);
            addField(db, fie);
            break;
        }
        case Path::MILESTONES_FOLDER:{
            MilestoneData mil(db, parentId);
            addMilestone(db, mil);
            break;
        }
        case Path::MILESTONE:{
            MilestoneStepData milStp(db, parentId);
            addMilestoneStep(db, milStp);
            break;
        }
        case Path::TEAM_FOLDER :{
            ProjectTeamMemberData member(db, pth.getIdByType(Path::PROJECT));
            addTeamMember(db, member);
            break;
        }
        case Path::ASSIGNMENTS_FOLDER :{
            AssignmentData assignment(db, parentId, pth.getIdByType(Path::PROJECT));
            addAssignment(db, assignment, pth.getIdByType(Path::PROJECT));
            break;
        }
        case Path::ASSIGNMENT :{
            HoursData hrs(db, parentId);
            addHours(db, hrs);
            break;
        }
        case Path::DATA_FOLDER :{
            ArtefactDataItemData dataItem(db, pth.getIdByType(Path::ARTEFACT), pth.getIdByType(Path::ARTEFACT_TYPE));
            addArtefactDataItem(db, dataItem);
            break;
        }
        case Path::ASSOCIATIONS_FOLDER :{
            ArtefactToActivityLinkData link(db, pth.getIdByType(Path::ARTEFACT), pth.getIdByType(Path::PROJECT));
            addArtefactToActivityLink(db, link);
            break;
        }
        case Path::ARTEFACT_TO_ACTIVITY_LINK : {
            MilestoneToArtefactLinkData link(db, parentId, pth.getIdByType(Path::PROJECT));
            addMilestoneToArtefactLink(db, link, pth.getIdByType(Path::PROJECT));
        }
    }
  } catch (const UserCancelledError&) {
        std::cout << "Add operation cancelled." << std::endl;
  } catch (const DataError& e) {
        throw;
  }
}

void add(Database& db, Path& pth, const std::string& csvValues) {
  
  try {
    int parentId = pth.current_id();
    std::vector<std::map<std::string,std::string>> CSVData;
    CSVData = utils::parseCSVString(csvValues);
    switch (pth.current_type()) {
        case Path::ROOT:
            throw NavigationError("Cannot add an item here.");
        case Path::ORGANIZATION_FOLDER:
            throw NavigationError("Cannot add an item here.");
        case Path::DEPARTMENTS_FOLDER :{
            DepartmentData dept(db, CSVData);
            addDepartment(db, dept);
            break;
        }
        case Path::DEPARTMENT:{
            EmployeeData emp(db, CSVData ,parentId);
            addEmployee(db, emp);
            break;
        }
        case Path::PROJECTS_FOLDER:{
            ProjectData proj(db, CSVData);
            addProject(db, proj);
            break;
        }
        case Path::PROJECT:
            throw NavigationError("Cannot add an item here.");
        case Path::ACTIVITIES_FOLDER:{
            CategoryData cat(db, CSVData, parentId);
            addCategory(db, cat, parentId);
            break;
        }
        case Path::CATEGORY:{
            SubCategoryData subCat(db, CSVData, parentId);
            addSubcategory(db, subCat, parentId);
            break;
        }
        case Path::SUBCATEGORY:{
            ActivityData act(db, CSVData, parentId);
            addActivity(db, act, parentId, pth.getIdByType(Path::PROJECT));
            break;
        }
        case Path::ACTIVITY:
            throw NavigationError("Cannot add an item here.");
        case Path::TASKS_FOLDER:{
            int actId;
            // Check if we're in a sub-tasks folder (under a task) or main tasks folder (under an activity)
            int parentTaskId = pth.getIdByType(Path::TASK);
            if (parentTaskId > 0) {
                // We're in a sub-tasks folder - get the activity ID for this task
                auto taskStmt = db.prepareStatement("SELECT activityid FROM ActivityTasks WHERE taskid = ?");
                taskStmt->setInt(1, parentTaskId);
                auto taskRes = taskStmt->executeQuery();
                if (taskRes->next()) {
                    actId = taskRes->getInt("activityid");
                    TaskData tsk(db, CSVData, actId, parentTaskId);
                    addTask(db, tsk, actId, parentTaskId);
                } else {
                    throw EntityNotFoundError("Activity", "for current task");
                }
            } else {
                // We're in the main tasks folder under an activity
                TaskData tsk(db, CSVData, parentId, 0);
                addTask(db, tsk, parentId, 0);
            }
            break;
        }
        case Path::TASK:
            throw NavigationError("Cannot add an item here. Navigate to Tasks folder first.");
        case Path::NOTES_FOLDER :{
            int projectId = pth.getIdByType(Path::PROJECT);
            NoteData noe(db, CSVData, parentId);
            addNote(db, noe, projectId);
            break;
        }
        case Path::ARTEFACT_TYPES_FOLDER :{
            ArtefactTypeData artTyp(db, CSVData, parentId);
            addArtefactType(db, artTyp);
            break;
        }
        case Path::ARTEFACT_TYPE:
            throw NavigationError("Cannot add an item here.");
        case Path::ARTEFACTS_FOLDER:{
            int projectId = pth.getIdByType(Path::PROJECT);
            ArtefactData art(db, CSVData, parentId);
            addArtefact(db, art, projectId);
            break;
        }
        case Path::FIELDS_FOLDER:{
            FieldData fie(db, CSVData, parentId);
            addField(db, fie);
            break;
        }
        case Path::MILESTONES_FOLDER:{
            MilestoneData mil(db, CSVData, parentId);
            addMilestone(db, mil);
            break;
        }
        case Path::MILESTONE:{
            MilestoneStepData milStp(db, CSVData, parentId);
            addMilestoneStep(db, milStp);
            break;
        }
        case Path::TEAM_FOLDER :{
            int projectId = pth.getIdByType(Path::PROJECT);
            ProjectTeamMemberData member(db, CSVData, projectId);
            addTeamMember(db, member);
            break;
        }
        case Path::ASSIGNMENTS_FOLDER :{
            int taskId = parentId;
            int projectId = pth.getIdByType(Path::PROJECT);
            AssignmentData assignment(db, CSVData, taskId);
            addAssignment(db, assignment, projectId);
            break;
        }
        case Path::ASSIGNMENT :{
            int assignmentId = parentId;
            HoursData hrs(db, CSVData, assignmentId);
            addHours(db, hrs);
            break;
        }
        case Path::DATA_FOLDER :{
            int artefactId = pth.getIdByType(Path::ARTEFACT);
            int artefactTypeId = pth.getIdByType(Path::ARTEFACT_TYPE);
            ArtefactDataItemData dataItem(db, CSVData, artefactId, artefactTypeId);
            addArtefactDataItem(db, dataItem);
            break;
        }
        case Path::ASSOCIATIONS_FOLDER :{
            int artefactId = pth.getIdByType(Path::ARTEFACT);
            int projectId = pth.getIdByType(Path::PROJECT);
            ArtefactToActivityLinkData link(db, CSVData, artefactId, projectId);
            addArtefactToActivityLink(db, link);
            break;
        }
        case Path::ARTEFACT_TO_ACTIVITY_LINK : {
            int artefactLinkId = parentId;
            int projectId = pth.getIdByType(Path::PROJECT);
            MilestoneToArtefactLinkData link(db, CSVData, artefactLinkId, projectId);
            addMilestoneToArtefactLink(db, link, projectId);
        }
    }
  } catch (const CsvParseError& e) {
        // Provide context-specific format help based on current path type
        std::string formatHelp;
        switch (pth.current_type()) {
            case Path::DEPARTMENT:
                formatHelp = "ad <firstname>,<lastname>,<email>,<password>";
                break;
            case Path::PROJECTS_FOLDER:
                formatHelp = "ad <projectname>,<projectno>";
                break;
            case Path::ACTIVITIES_FOLDER:
                formatHelp = "ad <categoryname>";
                break;
            case Path::CATEGORY:
                formatHelp = "ad <subcategoryname>";
                break;
            case Path::SUBCATEGORY:
                formatHelp = "ad <activityname>,<managerid>,<plannedstart>,<plannedfinish>,<plannedhours>";
                break;
            case Path::TASKS_FOLDER:
                formatHelp = "ad <taskname>";
                break;
            case Path::NOTES_FOLDER:
                formatHelp = "ad <userid>,<note>,<notedate>";
                break;
            case Path::ARTEFACT_TYPES_FOLDER:
                formatHelp = "ad <artefactname>,<artefactdescription>";
                break;
            case Path::ARTEFACTS_FOLDER:
                formatHelp = "ad <artefacttitle>,<artefactownerid>";
                break;
            case Path::FIELDS_FOLDER:
                formatHelp = "ad <fieldtitle>,<valuetype>,<maximumlength>,<maximumvalue>,<minimumvalue>";
                break;
            case Path::MILESTONES_FOLDER:
                formatHelp = "ad <milestonename>";
                break;
            case Path::MILESTONE:
                formatHelp = "ad <milestonestepname>,<progressratio>";
                break;
            case Path::DEPARTMENTS_FOLDER:
                formatHelp = "ad <departmentname>";
                break;
            case Path::TEAM_FOLDER:
                formatHelp = "ad <employeeid>,<role>";
                break;
            case Path::ASSIGNMENTS_FOLDER:
                formatHelp = "ad <userid>,<assigneddate>,<closedate>";
                break;
            case Path::ASSIGNMENT:
                formatHelp = "ad <bookeddate>,<hours>";
                break;
            case Path::DATA_FOLDER:
                formatHelp = "ad <artefactdatafieldid>,<value>";
                break;
            case Path::ASSOCIATIONS_FOLDER:
                formatHelp = "ad <activityid>,<ratio>";
                break;
            case Path::ARTEFACT_TO_ACTIVITY_LINK:
                formatHelp = "ad <milestonestepid>,<completedbyid>,<completiondate>";
                break;
            default:
                formatHelp = "ad <field1>,<field2>,...";
                break;
        }
        throw ValidationError("Invalid format. Use: " + formatHelp);
  } catch (const DataError& e) {
        throw;
  }
}


// Helper function to check if an employee is a member of the project team
static void checkProjectTeamMembership(Database& db, int employeeId, int projectId) {
    if (projectId <= 0) return;  // Skip check if projectId not provided
    auto stmt = db.prepareStatement(
        "SELECT teammemberid FROM ProjectTeamMembers WHERE projectid = ? AND employeeid = ?"
    );
    stmt->setInt(1, projectId);
    stmt->setInt(2, employeeId);
    auto res = stmt->executeQuery();
    if (!res->next()) {
        auto empStmt = db.prepareStatement(
            "SELECT CONCAT(firstname, ' ', lastname) as name FROM Employees WHERE employeeid = ?"
        );
        empStmt->setInt(1, employeeId);
        auto empRes = empStmt->executeQuery();
        std::string empName;
        if (empRes->next()) {
            empName = empRes->getString("name");
        } else {
            empName = std::to_string(employeeId);
        }
        throw ValidationError("Employee '" + empName + "' is not a member of the project team.");
    }
}
int addProject(Database& db, ProjectData& proj) {
    try {
        int count = 0;
        int lastId = 0;
        db.beginTransaction();
        do {
            auto insertStmt = db.prepareStatement(
                "INSERT INTO Projects (projectname, projectno) VALUES (?, ?)"
            );
            insertStmt->setString(1, proj.getName());
            insertStmt->setString(2, proj.getNumber());
            insertStmt->executeUpdate();
            lastId = db.getLastInsertId();
            count++;
        } while (proj.next());

        if (!count) {
            db.rollback();
            throw DataError("Error. No entries found.");
        }
        db.commit();
        std::cout << "Successfully added " << count << " project" << (count > 1 ? "s" : " : " + proj.getName())  << std::endl;
        return lastId;
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("creating project", e.what());
    }
}

int addCategory(Database& db, CategoryData& cat, int projectId){
    try{
        // Verify if project exists
        auto checkStmt = db.prepareStatement("SELECT projectid FROM Projects WHERE projectid = ?");
        checkStmt->setInt(1, projectId);
        auto checkRes = checkStmt->executeQuery();

        if(!checkRes->next()) {
            throw EntityNotFoundError("Project", projectId);
        }

        int count = 0;
        int lastId = 0;
        db.beginTransaction();
        do {
            auto insertStmt = db.prepareStatement(
                "INSERT INTO ActivityCategories (projectid, categoryname) VALUES (?, ?)"
            );
            insertStmt->setInt(1, projectId);
            insertStmt->setString(2, cat.getName());
            insertStmt->executeUpdate();
            lastId = db.getLastInsertId();
            count++;
        } while (cat.next());

        if (!count) {
            db.rollback();
            throw DataError("Error. No entries found.");
        }
        db.commit();
        std::cout << "Successfully added " << count << " categor" << (count > 1 ? "ies" : "y : " + cat.getName()) << std::endl;
        return lastId;
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("creating activity category", e.what());
    }
}

int addSubcategory(Database& db, SubCategoryData& subCat, int categoryId){
    try{
        // Verify if category exists
        auto checkStmt = db.prepareStatement("SELECT categoryid FROM ActivityCategories WHERE categoryid = ?");
        checkStmt->setInt(1, categoryId);
        auto checkRes = checkStmt->executeQuery();

        if(!checkRes->next()) {
            throw EntityNotFoundError("Activity category", categoryId);
        }

        int count = 0;
        int lastId = 0;
        db.beginTransaction();
        do {
            auto insertStmt = db.prepareStatement(
                "INSERT INTO ActivitySubcategories (categoryid, subcategoryname) VALUES (?, ?)"
            );
            insertStmt->setInt(1, categoryId);
            insertStmt->setString(2, subCat.getName());
            insertStmt->executeUpdate();
            lastId = db.getLastInsertId();
            count++;
        } while (subCat.next());

        if (!count) {
            db.rollback();
            throw DataError("Error. No entries found.");
        }
        db.commit();
        std::cout << "Successfully added " << count << " sub-categor" << (count > 1 ? "ies" : "y : " + subCat.getName()) << std::endl;
        return lastId;
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("creating activity sub-category", e.what());
    }
}
int addActivity(Database& db, ActivityData& act, int subcategoryId, int projectId) {
    try {
        // Verify if subcategory exists
        auto checkStmt = db.prepareStatement("SELECT subcategoryid FROM ActivitySubcategories WHERE subcategoryid = ?");
        checkStmt->setInt(1, subcategoryId);
        auto checkRes = checkStmt->executeQuery();

        if(!checkRes->next()) {
            throw EntityNotFoundError("Activity subcategory", subcategoryId);
        }

        int count = 0;
        int lastId = 0;
        db.beginTransaction();
        do {
            // Verify if activity manager exists
            auto empStmt = db.prepareStatement("SELECT employeeid FROM Employees WHERE employeeid = ?");
            empStmt->setInt(1, act.getManagerId());
            auto empRes = empStmt->executeQuery();

            if(!empRes->next()) {
                db.rollback();
                throw EntityNotFoundError("Employee", act.getManagerId());
            }

            // Verify activity manager is part of the project team
            checkProjectTeamMembership(db, act.getManagerId(), projectId);

            auto insertStmt = db.prepareStatement(
                "INSERT INTO Activities (subcategoryid, activityname, activitymanagerid, "
                "plannedstart, plannedfinish, plannedhours) VALUES (?, ?, ?, ?, ?, ?)"
            );
            insertStmt->setInt(1, subcategoryId);
            insertStmt->setString(2, act.getName());
            insertStmt->setInt(3, act.getManagerId());

            // Handle optional date/hour fields - use NULL if empty
            if (act.getPlannedStart().empty()) {
                insertStmt->setNull(4, sql::DataType::DATE);
            } else {
                insertStmt->setString(4, act.getPlannedStart());
            }
            if (act.getPlannedEnd().empty()) {
                insertStmt->setNull(5, sql::DataType::DATE);
            } else {
                insertStmt->setString(5, act.getPlannedEnd());
            }
            if (act.getPlannedHours() < 0) {
                insertStmt->setNull(6, sql::DataType::DECIMAL);
            } else {
                insertStmt->setDouble(6, act.getPlannedHours());
            }

            insertStmt->executeUpdate();
            lastId = db.getLastInsertId();
            count++;
        } while (act.next());

        if (!count) {
            db.rollback();
            throw DataError("Error. No entries found.");
        }
        db.commit();
        std::cout << "Successfully added " << count << " activit" << (count > 1 ? "ies" : "y : " + act.getName()) << std::endl;
        return lastId;
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("creating activity", e.what());
    }
}


int addTask(Database& db, TaskData& tas, int activityId, int parentTaskId) {
    try {
        // Verify activity exists
        auto checkStmt = db.prepareStatement("SELECT activityid FROM Activities WHERE activityid = ?");
        checkStmt->setInt(1, activityId);
        auto checkRes = checkStmt->executeQuery();

        if (!checkRes->next()) {
            throw EntityNotFoundError("Activity", activityId);
        }

        // If parent task specified, verify it exists and belongs to same activity
        if (parentTaskId > 0) {
            auto parentStmt = db.prepareStatement(
                "SELECT taskid FROM ActivityTasks WHERE taskid = ? AND activityid = ?"
            );
            parentStmt->setInt(1, parentTaskId);
            parentStmt->setInt(2, activityId);
            auto parentRes = parentStmt->executeQuery();

            if (!parentRes->next()) {
                throw EntityNotFoundError("Parent task", parentTaskId);
            }
        }

        int count = 0;
        int lastId = 0;
        db.beginTransaction();
        do {
            auto insertStmt = db.prepareStatement(
                "INSERT INTO ActivityTasks (activityid, taskname,taskdescription, parenttaskid, departmentid) VALUES (?, ?, ?, ?, ?)"
            );
            insertStmt->setInt(1, activityId);
            insertStmt->setString(2, tas.getName());
            insertStmt->setString(3, tas.getDescription());

            if (parentTaskId > 0) {
                insertStmt->setInt(4, parentTaskId);
            } else {
                insertStmt->setNull(4, sql::DataType::INTEGER);
            }

            if (tas.getDepartmentId() > 0) {
                insertStmt->setInt(5, tas.getDepartmentId());
            } else {
                insertStmt->setNull(5, sql::DataType::INTEGER);
            }

            insertStmt->executeUpdate();
            lastId = db.getLastInsertId();
            count++;
        } while (tas.next());

        if (!count) {
            db.rollback();
            throw DataError("Error. No entries found.");
        }
        db.commit();
        std::cout << "Successfully added " << count << " task" << (count > 1 ? "s" : " : " + tas.getName()) << std::endl;
        return lastId;
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("creating task", e.what());
    }
}

int addEmployee(Database& db, EmployeeData& emp) {
    try {
        int count = 0;
        int lastId = 0;
        db.beginTransaction();
        do {
            // Generate salt and hash password
            std::string salt = auth::generateSalt();
            std::string hashedPassword = auth::hashPassword(emp.getPassword(), salt);

            auto insertStmt = db.prepareStatement(
                "INSERT INTO Employees (departmentid, firstname, lastname, email, password, passwordsalt, admin) VALUES (?, ?, ?, ?, ?, ?, ?)"
            );
            insertStmt->setInt(1, emp.getDepartmentId());
            insertStmt->setString(2, emp.getFirstName());
            insertStmt->setString(3, emp.getLastName());
            insertStmt->setString(4, emp.getEmail());
            insertStmt->setString(5, hashedPassword);
            insertStmt->setString(6, salt);
            insertStmt->setBoolean(7, emp.isAdmin());
            insertStmt->executeUpdate();
            lastId = db.getLastInsertId();
            count++;
        } while (emp.next());

        if (!count) {
            db.rollback();
            throw DataError("Error. No entries found.");
        }
        db.commit();
        std::cout << "Successfully added " << count << " employee" << (count > 1 ? "s" : " : " + emp.getFirstName() + " " + emp.getLastName()) << std::endl;
        return lastId;
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("creating employee", e.what());
    }
}

int addArtefactType(Database& db, ArtefactTypeData& artTyp) {
    try {
        // Verify if project exists
        auto checkStmt = db.prepareStatement("SELECT projectid FROM Projects WHERE projectid = ?");
        checkStmt->setInt(1, artTyp.getProjectId());
        auto checkRes = checkStmt->executeQuery();

        if(!checkRes->next()) {
            throw EntityNotFoundError("Project", artTyp.getProjectId());
        }

        int count = 0;
        int lastId = 0;
        db.beginTransaction();
        do {
            auto insertStmt = db.prepareStatement(
                "INSERT INTO ArtefactTypes (projectid, artefactname, artefactdescription) VALUES (?, ?, ?)"
            );
            insertStmt->setInt(1, artTyp.getProjectId());
            insertStmt->setString(2, artTyp.getName());
            insertStmt->setString(3, artTyp.getDescription());
            insertStmt->executeUpdate();
            lastId = db.getLastInsertId();
            count++;
        } while (artTyp.next());

        if (!count) {
            db.rollback();
            throw DataError("Error. No entries found.");
        }
        db.commit();
        std::cout << "Successfully added " << count << " artefact type" << (count > 1 ? "s" : " : " + artTyp.getName()) << std::endl;
        return lastId;
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("creating artefact type", e.what());
    }
}

int addArtefact(Database& db, ArtefactData& art, int projectId) {
    try {
        // Verify if artefact type exists
        auto checkStmt = db.prepareStatement("SELECT artefacttypeid FROM ArtefactTypes WHERE artefacttypeid = ?");
        checkStmt->setInt(1, art.getArtefactTypeId());
        auto checkRes = checkStmt->executeQuery();

        if(!checkRes->next()) {
            throw EntityNotFoundError("Artefact type", art.getArtefactTypeId());
        }

        int count = 0;
        int lastId = 0;
        db.beginTransaction();
        do {
            // Verify if artefact owner exists
            auto empStmt = db.prepareStatement("SELECT employeeid FROM Employees WHERE employeeid = ?");
            empStmt->setInt(1, art.getArtefactOwnerId());
            auto empRes = empStmt->executeQuery();

            if(!empRes->next()) {
                db.rollback();
                throw EntityNotFoundError("Employee", art.getArtefactOwnerId());
            }

            // Verify artefact owner is part of the project team
            checkProjectTeamMembership(db, art.getArtefactOwnerId(), projectId);

            auto insertStmt = db.prepareStatement(
                "INSERT INTO Artefacts (artefacttypeid, artefacttitle, artefactownerid) VALUES (?, ?, ?)"
            );
            insertStmt->setInt(1, art.getArtefactTypeId());
            insertStmt->setString(2, art.getName());
            insertStmt->setInt(3, art.getArtefactOwnerId());
            insertStmt->executeUpdate();
            lastId = db.getLastInsertId();
            count++;
        } while (art.next());

        if (!count) {
            db.rollback();
            throw DataError("Error. No entries found.");
        }
        db.commit();
        std::cout << "Successfully added " << count << " artefact" << (count > 1 ? "s" : " : " + art.getName()) << std::endl;
        return lastId;
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("creating artefact", e.what());
    }
}

int addNote(Database& db, NoteData& noe, int projectId) {
    try {
        // Verify if activity exists
        auto checkStmt = db.prepareStatement("SELECT activityid FROM Activities WHERE activityid = ?");
        checkStmt->setInt(1, noe.getActivityId());
        auto checkRes = checkStmt->executeQuery();

        if(!checkRes->next()) {
            throw EntityNotFoundError("Activity", noe.getActivityId());
        }

        int count = 0;
        int lastId = 0;
        db.beginTransaction();
        do {
            // Verify if employee exists
            auto empStmt = db.prepareStatement("SELECT employeeid FROM Employees WHERE employeeid = ?");
            empStmt->setInt(1, noe.getUserId());
            auto empRes = empStmt->executeQuery();

            if(!empRes->next()) {
                db.rollback();
                throw EntityNotFoundError("Employee", noe.getUserId());
            }

            // Verify note author is part of the project team
            checkProjectTeamMembership(db, noe.getUserId(), projectId);

            auto insertStmt = db.prepareStatement(
                "INSERT INTO ActivityNotes (activityid, userid, note, notedate) VALUES (?, ?, ?, ?)"
            );
            insertStmt->setInt(1, noe.getActivityId());
            insertStmt->setInt(2, noe.getUserId());
            insertStmt->setString(3, noe.getNote());
            insertStmt->setString(4, noe.getNoteDate());
            insertStmt->executeUpdate();
            lastId = db.getLastInsertId();
            count++;
        } while (noe.next());

        if (!count) {
            db.rollback();
            throw DataError("Error. No entries found.");
        }
        db.commit();
        std::cout << "Successfully added " << count << " note" << (count > 1 ? "s" : " : " + noe.getNote()) << std::endl;
        return lastId;
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("creating note", e.what());
    }
}

int addField(Database& db, FieldData& fie){
    try {
        // Verify if artefact type exists
        auto checkStmt = db.prepareStatement("SELECT artefacttypeid FROM ArtefactTypes WHERE artefacttypeid = ?");
        checkStmt->setInt(1, fie.getArtefactTypeId());
        auto checkRes = checkStmt->executeQuery();

        if(!checkRes->next()) {
            throw EntityNotFoundError("Artefact type", fie.getArtefactTypeId());
        }

        int count = 0;
        int lastId = 0;
        db.beginTransaction();
        do {
            auto insertStmt = db.prepareStatement(
                "INSERT INTO ArtefactDataFields (artefacttypeid, fieldtitle, valuetype, maximumlength, maximumvalue, minimumvalue) VALUES (?, ?, ?, ?, ?, ?)"
            );
            insertStmt->setInt(1, fie.getArtefactTypeId());
            insertStmt->setString(2, fie.getTitle());
            insertStmt->setInt(3, fie.getValueType());

            // Set maximumlength (only for text fields)
            if (fie.getValueType() == 0 && fie.getMaimumLength() > 0) {
                insertStmt->setInt(4, fie.getMaimumLength());
            } else {
                insertStmt->setNull(4, sql::DataType::INTEGER);
            }

            // Set maximumvalue and minimumvalue (only for numeric fields)
            if (fie.getValueType() == 1) {
                if (fie.getMaximumValue() > 0) {
                    insertStmt->setInt(5, fie.getMaximumValue());
                } else {
                    insertStmt->setNull(5, sql::DataType::INTEGER);
                }
                if (fie.getMinimumValue() > 0) {
                    insertStmt->setInt(6, fie.getMinimumValue());
                } else {
                    insertStmt->setNull(6, sql::DataType::INTEGER);
                }
            } else {
                insertStmt->setNull(5, sql::DataType::INTEGER);
                insertStmt->setNull(6, sql::DataType::INTEGER);
            }

            insertStmt->executeUpdate();
            lastId = db.getLastInsertId();
            count++;
        } while (fie.next());

        if (!count) {
            db.rollback();
            throw DataError("No entries found.");
        }
        db.commit();
        std::cout << "Successfully added " << count << " field" << (count > 1 ? "s" : " : " + fie.getTitle()) << std::endl;
        return lastId;
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("creating field", e.what());
    }
}

int addMilestone(Database& db, MilestoneData& mil){
    try {
        // Verify if project exists
        auto checkStmt = db.prepareStatement("SELECT projectid FROM Projects WHERE projectid = ?");
        checkStmt->setInt(1, mil.getProjectId());
        auto checkRes = checkStmt->executeQuery();

        if(!checkRes->next()) {
            throw EntityNotFoundError("Project", mil.getProjectId());
        }

        int count = 0;
        int lastId = 0;
        db.beginTransaction();
        do {
            auto insertStmt = db.prepareStatement(
                "INSERT INTO Milestones (projectid, milestonename) VALUES (?, ?)"
            );
            insertStmt->setInt(1, mil.getProjectId());
            insertStmt->setString(2, mil.getName());
            insertStmt->executeUpdate();
            lastId = db.getLastInsertId();
            count++;
        } while (mil.next());

        if (!count) {
            db.rollback();
            throw DataError("No entries found.");
        }
        db.commit();
        std::cout << "Successfully added " << count << " milestone" << (count > 1 ? "s" : " : " + mil.getName()) << std::endl;
        return lastId;
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("creating milestone", e.what());
    }
}

int addMilestoneStep(Database& db, MilestoneStepData& milStp){
    try {
        // Verify if milestone exists
        auto checkStmt = db.prepareStatement("SELECT milestoneid FROM Milestones WHERE milestoneid = ?");
        checkStmt->setInt(1, milStp.getMilestoneId());
        auto checkRes = checkStmt->executeQuery();

        if(!checkRes->next()) {
            throw EntityNotFoundError("Milestone", milStp.getMilestoneId());
        }

        int count = 0;
        int lastId = 0;
        db.beginTransaction();
        do {
            auto insertStmt = db.prepareStatement(
                "INSERT INTO MilestoneSteps (milestoneid, milestonestepname, progressratio) VALUES (?, ?, ?)"
            );
            insertStmt->setInt(1, milStp.getMilestoneId());
            insertStmt->setString(2, milStp.getName());
            insertStmt->setDouble(3, milStp.getProgressRatio());
            insertStmt->executeUpdate();
            lastId = db.getLastInsertId();
            count++;
        } while (milStp.next());

        if (!count) {
            db.rollback();
            throw DataError("No entries found.");
        }
        db.commit();
        std::cout << "Successfully added " << count << " milestone step" << (count > 1 ? "s" : " : " + milStp.getName()) << std::endl;
        return lastId;
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("creating milestone step", e.what());
    }
}

int addDepartment(Database& db, DepartmentData& dept) {
    try {
        int count = 0;
        int lastId = 0;
        db.beginTransaction();
        do {
            auto insertStmt = db.prepareStatement(
                "INSERT INTO Departments (departmentname) VALUES (?)"
            );
            insertStmt->setString(1, dept.getName());
            insertStmt->executeUpdate();
            lastId = db.getLastInsertId();
            count++;
        } while (dept.next());

        if (!count) {
            db.rollback();
            throw DataError("No entries found.");
        }
        db.commit();
        std::cout << "Successfully added " << count << " department" << (count > 1 ? "s" : " : " + dept.getName()) << std::endl;
        return lastId;
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("creating department", e.what());
    }
}

int addTeamMember(Database& db, ProjectTeamMemberData& member) {
    try {
        auto checkProject = db.prepareStatement("SELECT projectid FROM Projects WHERE projectid = ?");
        checkProject->setInt(1, member.getProjectId());
        auto checkRes = checkProject->executeQuery();
        if (!checkRes->next()) {
            throw EntityNotFoundError("Project", member.getProjectId());
        }

        int count = 0;
        int lastId = 0;
        db.beginTransaction();
        do {
            auto empCheck = db.prepareStatement("SELECT employeeid FROM Employees WHERE employeeid = ?");
            empCheck->setInt(1, member.getEmployeeId());
            auto empRes = empCheck->executeQuery();
            if (!empRes->next()) {
                db.rollback();
                throw EntityNotFoundError("Employee", member.getEmployeeId());
            }

            auto insertStmt = db.prepareStatement(
                "INSERT INTO ProjectTeamMembers (projectid, employeeid, role) VALUES (?, ?, ?)"
            );
            insertStmt->setInt(1, member.getProjectId());
            insertStmt->setInt(2, member.getEmployeeId());
            insertStmt->setInt(3, member.getRole());
            insertStmt->executeUpdate();
            lastId = db.getLastInsertId();
            count++;
        } while (member.next());

        if (!count) {
            db.rollback();
            throw DataError("No entries found.");
        }
        db.commit();
        std::cout << "Successfully added " << count << " team member" << (count > 1 ? "s" : "") << std::endl;
        return lastId;
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("creating team member", e.what());
    }
}

int addAssignment(Database& db, AssignmentData& assignment, int projectId) {
    try {
        auto checkTask = db.prepareStatement("SELECT taskid FROM ActivityTasks WHERE taskid = ?");
        checkTask->setInt(1, assignment.getTaskId());
        auto checkRes = checkTask->executeQuery();
        if (!checkRes->next()) {
            throw EntityNotFoundError("Task", assignment.getTaskId());
        }

        int count = 0;
        int lastId = 0;
        db.beginTransaction();
        do {
            auto empCheck = db.prepareStatement("SELECT employeeid FROM Employees WHERE employeeid = ?");
            empCheck->setInt(1, assignment.getUserId());
            auto empRes = empCheck->executeQuery();
            if (!empRes->next()) {
                db.rollback();
                throw EntityNotFoundError("Employee", assignment.getUserId());
            }

            // Verify assigned user is part of the project team
            checkProjectTeamMembership(db, assignment.getUserId(), projectId);

            auto insertStmt = db.prepareStatement(
                "INSERT INTO ActivityTaskAssignments (taskid, userid, assigneddate, closedate) VALUES (?, ?, ?, ?)"
            );
            insertStmt->setInt(1, assignment.getTaskId());
            insertStmt->setInt(2, assignment.getUserId());
            insertStmt->setString(3, assignment.getAssignedDate());
            if (!assignment.getCloseDate().empty()) {
                insertStmt->setString(4, assignment.getCloseDate());
            } else {
                insertStmt->setNull(4, sql::DataType::DATE);
            }
            insertStmt->executeUpdate();
            lastId = db.getLastInsertId();
            count++;
        } while (assignment.next());

        if (!count) {
            db.rollback();
            throw DataError("No entries found.");
        }
        db.commit();
        std::cout << "Successfully added " << count << " user assignment entr" << (count > 1 ? "ies" : "y") << std::endl;
        return lastId;
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("creating assignment", e.what());
    }
}

int addHours(Database& db, HoursData& hours) {
    try {
        auto checkAssignment = db.prepareStatement("SELECT assignmentid FROM ActivityTaskAssignments WHERE assignmentid = ?");
        checkAssignment->setInt(1, hours.getAssignmentId());
        auto checkRes = checkAssignment->executeQuery();
        if (!checkRes->next()) {
            throw EntityNotFoundError("Assignment", hours.getAssignmentId());
        }

        int count = 0;
        int lastId = 0;
        db.beginTransaction();
        do {
            auto insertStmt = db.prepareStatement(
                "INSERT INTO Hours (assignmentid, bookeddate, hours) VALUES (?, ?, ?)"
            );
            insertStmt->setInt(1, hours.getAssignmentId());
            insertStmt->setString(2, hours.getBookedDate());
            insertStmt->setInt(3, hours.getHours());
            insertStmt->executeUpdate();
            lastId = db.getLastInsertId();
            count++;
        } while (hours.next());

        if (!count) {
            db.rollback();
            throw DataError("No entries found.");
        } else {
            db.commit();
            std::cout << "Successfully added " << count << " hour booking entr" << (count > 1 ? "ies" : "y") << std::endl;
        }
        return lastId;
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("creating hours booking", e.what());
    }
}

int addArtefactDataItem(Database& db, ArtefactDataItemData& data) {
    try {
        auto checkArtefact = db.prepareStatement("SELECT artefactid FROM Artefacts WHERE artefactid = ?");
        checkArtefact->setInt(1, data.getArtefactId());
        auto checkRes = checkArtefact->executeQuery();
        if (!checkRes->next()) {
            throw EntityNotFoundError("Artefact", data.getArtefactId());
        }

        int count = 0;
        int lastId = 0;
        db.beginTransaction();
        do {
            auto fieldCheck = db.prepareStatement("SELECT artefactdatafieldid FROM ArtefactDataFields WHERE artefactdatafieldid = ?");
            fieldCheck->setInt(1, data.getArtefactDataFieldId());
            auto fieldRes = fieldCheck->executeQuery();
            if (!fieldRes->next()) {
                db.rollback();
                throw EntityNotFoundError("Field", data.getArtefactDataFieldId());
            }

            auto insertStmt = db.prepareStatement(
                "INSERT INTO ArtefactData (artefactid, artefactdatafieldid, value) VALUES (?, ?, ?)"
            );
            insertStmt->setInt(1, data.getArtefactId());
            insertStmt->setInt(2, data.getArtefactDataFieldId());
            insertStmt->setString(3, data.getValue());
            insertStmt->executeUpdate();
            lastId = db.getLastInsertId();
            count++;
        } while (data.next());

        if (!count) {
            db.rollback();
            throw DataError("No entries found.");
        }
        db.commit();
        std::cout << "Successfully added " << count << " artefact data entr" << (count > 1 ? "ies" : "y") << std::endl;
        return lastId;
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("creating artefact data", e.what());
    }
}

int addArtefactToActivityLink(Database& db, ArtefactToActivityLinkData& link) {
    try {
        auto checkArtefact = db.prepareStatement("SELECT artefactid FROM Artefacts WHERE artefactid = ?");
        checkArtefact->setInt(1, link.getArtefactId());
        auto checkRes = checkArtefact->executeQuery();
        if (!checkRes->next()) {
            throw EntityNotFoundError("Artefact", link.getArtefactId());
        }

        int count = 0;
        int lastId = 0;
        db.beginTransaction();
        do {
            auto actCheck = db.prepareStatement("SELECT activityid FROM Activities WHERE activityid = ?");
            actCheck->setInt(1, link.getActivityId());
            auto actRes = actCheck->executeQuery();
            if (!actRes->next()) {
                db.rollback();
                throw EntityNotFoundError("Activity", link.getActivityId());
            }

            auto insertStmt = db.prepareStatement(
                "INSERT INTO ArtefactToActivityLink (activityid, artefactid, ratio) VALUES (?, ?, ?)"
            );
            insertStmt->setInt(1, link.getActivityId());
            insertStmt->setInt(2, link.getArtefactId());
            insertStmt->setDouble(3, link.getRatio());
            insertStmt->executeUpdate();
            lastId = db.getLastInsertId();
            count++;
        } while (link.next());

        if (!count) {
            db.rollback();
            throw DataError("No entries found.");
        }
        db.commit();
        std::cout << "Successfully added " << count << " artefact-activity link entr" << (count > 1 ? "ies" : "y") << std::endl;
        return lastId;
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("creating artefact-activity link", e.what());
    }
}

int addMilestoneToArtefactLink(Database& db, MilestoneToArtefactLinkData& link, int projectId) {
    try {
        auto checkLink = db.prepareStatement("SELECT artefactlinkid FROM ArtefactToActivityLink WHERE artefactlinkid = ?");
        checkLink->setInt(1, link.getArtefactLinkId());
        auto checkRes = checkLink->executeQuery();
        if (!checkRes->next()) {
            throw EntityNotFoundError("Artefact link", link.getArtefactLinkId());
        }

        int count = 0;
        int lastId = 0;
        db.beginTransaction();
        do {
            auto stepCheck = db.prepareStatement("SELECT milestonestepid FROM MilestoneSteps WHERE milestonestepid = ?");
            stepCheck->setInt(1, link.getMilestoneStepId());
            auto stepRes = stepCheck->executeQuery();
            if (!stepRes->next()) {
                db.rollback();
                throw EntityNotFoundError("Milestone step", link.getMilestoneStepId());
            }

            // Verify completed by user is part of the project team
            if (link.getCompletedById() > 0) {
                checkProjectTeamMembership(db, link.getCompletedById(), projectId);
            }

            auto insertStmt = db.prepareStatement(
                "INSERT INTO MilestoneToArtefactLink (milestonestepid, artefactlinkid, completedbyid, completiondate) VALUES (?, ?, ?, ?)"
            );
            insertStmt->setInt(1, link.getMilestoneStepId());
            insertStmt->setInt(2, link.getArtefactLinkId());
            if (link.getCompletedById() > 0) {
                insertStmt->setInt(3, link.getCompletedById());
            } else {
                insertStmt->setNull(3, sql::DataType::INTEGER);
            }
            if (!link.getCompletionDate().empty()) {
                insertStmt->setString(4, link.getCompletionDate());
            } else {
                insertStmt->setNull(4, sql::DataType::DATE);
            }
            insertStmt->executeUpdate();
            lastId = db.getLastInsertId();
            count++;
        } while (link.next());

        if (!count) {
            db.rollback();
            throw DataError("No entries found.");
        }
        db.commit();
        std::cout << "Successfully added " << count << " milestone-artefact link entr" << (count > 1 ? "ies" : "y") << std::endl;
        return lastId;
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("creating milestone-artefact link", e.what());
    }
}

} // namespace commands
