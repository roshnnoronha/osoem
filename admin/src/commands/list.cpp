#include "list.h"

#include <iostream>
#include <iomanip>
#include <string>

namespace commands {

void list(Database& db, Path& pth, bool showAll){
    switch(pth.current_type()){
    case Path::ROOT:
        std::cout << "Folders:" << std::endl;
        if (showAll) {
            std::cout << std::string(70, '=') << std::endl;
            std::cout << std::left << std::setw(5) << "#" << "Folder" << std::endl;
            std::cout << std::string(70, '-') << std::endl;
            std::cout << std::left << std::setw(5) << "1" << "Organization" << std::endl;
            std::cout << std::left << std::setw(5) << "2" << "Projects" << std::endl;
            std::cout << std::string(70, '=') << std::endl;
        } else {
            std::cout << "(1) Organization" << std::endl;
            std::cout << "(2) Projects" << std::endl;
        }
        break;
    case Path::ORGANIZATION_FOLDER:
        std::cout << "Folders:" << std::endl;
        if (showAll) {
            std::cout << std::string(70, '=') << std::endl;
            std::cout << std::left << std::setw(5) << "#" << "Folder" << std::endl;
            std::cout << std::string(70, '-') << std::endl;
            std::cout << std::left << std::setw(5) << "1" << "Departments" << std::endl;
            std::cout << std::string(70, '=') << std::endl;
        } else {
            std::cout << "(1) Departments" << std::endl;
        }
        break;
    case Path::DEPARTMENTS_FOLDER:
        listDepartments(db, showAll);
        break;
    case Path::DEPARTMENT:
        listEmployees(db, pth.current_id(), showAll);
        break;
    case Path::PROJECTS_FOLDER:
        listProjects(db, showAll);
        break;
    case Path::PROJECT:
        std::cout << "Folders:" << std::endl;
        if (showAll) {
            std::cout << std::string(70, '=') << std::endl;
            std::cout << std::left << std::setw(5) << "#" << "Folder" << std::endl;
            std::cout << std::string(70, '-') << std::endl;
            std::cout << std::left << std::setw(5) << "1" << "Activity Categories" << std::endl;
            std::cout << std::left << std::setw(5) << "2" << "Artefact Types" << std::endl;
            std::cout << std::left << std::setw(5) << "3" << "Milestones" << std::endl;
            std::cout << std::left << std::setw(5) << "4" << "Team" << std::endl;
            std::cout << std::string(70, '=') << std::endl;
        } else {
            std::cout << "(1) Activity Categories" << std::endl;
            std::cout << "(2) Artefact Types" << std::endl;
            std::cout << "(3) Milestones" << std::endl;
            std::cout << "(4) Team" << std::endl;
        }
        break;
    case Path::TEAM_FOLDER:
        listTeamMembers(db, pth.getIdByType(Path::PROJECT), showAll);
        break;
    case Path::ACTIVITIES_FOLDER:
        listCategories(db, pth.current_id(), showAll);
        break;
    case Path::CATEGORY:
        listSubcategories(db, pth.current_id(), showAll);
        break;
    case Path::SUBCATEGORY:
        listActivities(db, pth.current_id(), showAll);
        break;
    case Path::ACTIVITY:
        std::cout << "Folders:" << std::endl;
        if (showAll) {
            std::cout << std::string(70, '=') << std::endl;
            std::cout << std::left << std::setw(5) << "#" << "Folder" << std::endl;
            std::cout << std::string(70, '-') << std::endl;
            std::cout << std::left << std::setw(5) << "1" << "Tasks" << std::endl;
            std::cout << std::left << std::setw(5) << "2" << "Notes" << std::endl;
            std::cout << std::string(70, '=') << std::endl;
        } else {
            std::cout << "(1) Tasks" << std::endl;
            std::cout << "(2) Notes" << std::endl;
        }
        break;
    case Path::TASKS_FOLDER:
        if (pth.getIdByType(Path::TASK) > 0) {
            listSubTasks(db, pth.current_id(), showAll);
        } else {
            listTasks(db, pth.current_id(), showAll);
        }
        break;
    case Path::TASK:
    case Path::SUBTASK:
        std::cout << "Folders:" << std::endl;
        if (showAll) {
            std::cout << std::string(70, '=') << std::endl;
            std::cout << std::left << std::setw(5) << "#" << "Folder" << std::endl;
            std::cout << std::string(70, '-') << std::endl;
            std::cout << std::left << std::setw(5) << "1" << "Tasks" << std::endl;
            std::cout << std::left << std::setw(5) << "2" << "Assignments" << std::endl;
            std::cout << std::string(70, '=') << std::endl;
        } else {
            std::cout << "(1) Tasks" << std::endl;
            std::cout << "(2) Assignments" << std::endl;
        }
        break;
    case Path::ASSIGNMENTS_FOLDER:
        listAssignments(db, pth.current_id(), showAll);
        break;
    case Path::ASSIGNMENT:
        listHours(db, pth.current_id(), showAll);
        break;
    case Path::NOTES_FOLDER:
        listNotes(db, pth.current_id(), showAll);
        break;
    case Path::ARTEFACT_TYPES_FOLDER:
        listArtefactTypes(db, pth.current_id(), showAll);
        break;
    case Path::ARTEFACT_TYPE:
        std::cout << "Folders:" << std::endl;
        if (showAll) {
            std::cout << std::string(70, '=') << std::endl;
            std::cout << std::left << std::setw(5) << "#" << "Folder" << std::endl;
            std::cout << std::string(70, '-') << std::endl;
            std::cout << std::left << std::setw(5) << "1" << "Artefacts" << std::endl;
            std::cout << std::left << std::setw(5) << "2" << "Fields" << std::endl;
            std::cout << std::string(70, '=') << std::endl;
        } else {
            std::cout << "(1) Artefacts" << std::endl;
            std::cout << "(2) Fields" << std::endl;
        }
        break;
    case Path::ARTEFACTS_FOLDER:
        listArtefacts(db, pth.current_id(), showAll);
        break;
    case Path::ARTEFACT:
        std::cout << "Folders:" << std::endl;
        if (showAll) {
            std::cout << std::string(70, '=') << std::endl;
            std::cout << std::left << std::setw(5) << "#" << "Folder" << std::endl;
            std::cout << std::string(70, '-') << std::endl;
            std::cout << std::left << std::setw(5) << "1" << "Data" << std::endl;
            std::cout << std::left << std::setw(5) << "2" << "Associations" << std::endl;
            std::cout << std::string(70, '=') << std::endl;
        } else {
            std::cout << "(1) Data" << std::endl;
            std::cout << "(2) Associations" << std::endl;
        }
        break;
    case Path::DATA_FOLDER:
        listArtefactDataItems(db, pth.getIdByType(Path::ARTEFACT), showAll);
        break;
    case Path::ASSOCIATIONS_FOLDER:
        listArtefactToActivityLinks(db, pth.getIdByType(Path::ARTEFACT), showAll);
        break;
    case Path::ARTEFACT_TO_ACTIVITY_LINK:
        listMilestoneToArtefactLinks(db, pth.current_id(), showAll);
        break;
    case Path::FIELDS_FOLDER:
        listFields(db, pth.current_id(), showAll);
        break;
    case Path::MILESTONES_FOLDER:
        listMilestones(db, pth.current_id(), showAll);
        break;
    case Path::MILESTONE:
        listMilestoneSteps(db, pth.current_id(), showAll);
        break;
    }
}

void listActivities(Database& db, int subCategoryId, bool showAll) {
    std::string query = R"(
        SELECT a.activityid, a.activityname, a.activitydescription,
               a.plannedstart, a.plannedfinish,
               a.forecaststart, a.forecastfinish,
               a.actualstart, a.actualfinish,
               a.plannedhours, a.forecasthours,
               a.activitymanagerid,
               CONCAT(e.firstname, ' ', e.lastname) as manager
        FROM Activities a
        JOIN Employees e ON a.activitymanagerid = e.employeeid
        JOIN ActivitySubcategories acs ON a.subcategoryid = acs.subcategoryid
        JOIN ActivityCategories ac ON acs.categoryid = ac.categoryid
    )";

    std::unique_ptr<sql::ResultSet> res;
    if (subCategoryId > 0) {
        query += " WHERE a.subcategoryid = ? ORDER BY a.activityid";
        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, subCategoryId);
        res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
    } else {
        query += " ORDER BY a.activityid";
        res = db.executeQuery(query);
    }

    int count = 0;
    std::cout << "Activities:" << std::endl;
    if (showAll) {
        std::cout << std::string(192, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(30) << "ActivityName"
                  << std::setw(25) << "ActivityDescription"
                  << std::setw(14) << "PlannedStart"
                  << std::setw(14) << "PlannedFinish"
                  << std::setw(14) << "ForecastStart"
                  << std::setw(14) << "ForecastFinish"
                  << std::setw(14) << "ActualFinish"
                  << std::setw(14) << "PlannedHours"
                  << std::setw(14) << "ForecastHours"
                  << std::setw(34) << "ActivityManagerId"
                  << std::endl;
        std::cout << std::string(192, '-') << std::endl;

        while (res->next()) {
            std::string desc = res->isNull("activitydescription") ? "" : res->getString("activitydescription");
            std::string pStart = res->isNull("plannedstart") ? "" : res->getString("plannedstart");
            std::string pFinish = res->isNull("plannedfinish") ? "" : res->getString("plannedfinish");
            std::string fStart = res->isNull("forecaststart") ? "" : res->getString("forecaststart");
            std::string fFinish = res->isNull("forecastfinish") ? "" : res->getString("forecastfinish");
            std::string aFinish = res->isNull("actualfinish") ? "" : res->getString("actualfinish");
            std::string pHours = res->isNull("plannedhours") ? "" : std::to_string(res->getDouble("plannedhours"));
            std::string fHours = res->isNull("forecasthours") ? "" : std::to_string(res->getDouble("forecasthours"));
            std::string manager = std::to_string(res->getInt("activitymanagerid")) + " (" + res->getString("manager") + ")";

            std::cout << std::left << std::setw(5) << res->getInt("activityid")
                      << std::setw(30) << res->getString("activityname").substr(0, 29)
                      << std::setw(25) << desc.substr(0, 24)
                      << std::setw(14) << pStart
                      << std::setw(14) << pFinish
                      << std::setw(14) << fStart
                      << std::setw(14) << fFinish
                      << std::setw(14) << aFinish
                      << std::setw(14) << pHours
                      << std::setw(14) << fHours
                      << std::setw(34) << manager.substr(0, 33)
                      << std::endl;
            count++;
        }
        std::cout << std::string(192, '=') << std::endl;
    } else {
        while (res->next()) {
            std::cout << "(" << res->getInt("activityid") << ") "
                      << res->getString("activityname") << std::endl;
            count++;
        }
    }
}

void listCategories(Database& db, int projectId, bool showAll){
    std::string query = R"(
        SELECT
            c.categoryid,
            c.categoryname
        FROM ActivityCategories c
    )";

    std::unique_ptr<sql::ResultSet> res;
    if (projectId > 0) {
        query += " WHERE projectid = ?";
        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, projectId);
        res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
    } else {
        res = db.executeQuery(query);
    }

    int count = 0;
    std::cout << "Activity Categories:" << std::endl;
    if (showAll) {
        std::cout << std::string(70,'=') <<'\n';
        std::cout << std::left << std::setw(5) << "ID"
                    <<std::setw(40) << "CategoryName" << std::endl;
        std::cout << std::string(70,'-') << std::endl;

        while (res->next()) {
            std::cout << std::left << std::setw(5) << res->getInt("categoryid")
                << std::setw(40) << res->getString("categoryname").substr(0,39)
                << std::endl;
            count ++;
        }
        std::cout << std::string(70, '=') << std::endl;
    } else {
        while (res->next()) {
            std::cout << "(" << res->getInt("categoryid") << ") "
                      << res->getString("categoryname") << std::endl;
            count++;
        }
    }
}

void listSubcategories(Database& db, int categoryId, bool showAll){
    std::string query = R"(
        SELECT s.subcategoryid, s.subcategoryname
        FROM ActivitySubcategories s
        WHERE categoryid = ?
    )";
    auto stmt = db.prepareStatement(query);
    stmt->setInt(1, categoryId);
    auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

    int count = 0;
    std::cout << "Activity Sub-categories:" << std::endl;
    if (showAll) {
        std::cout << std::string(70,'=') <<'\n';
        std::cout << std::left << std::setw(5) << "ID"
                    <<std::setw(40) << "SubCategoryName" << std::endl;
        std::cout << std::string(70,'-') << std::endl;

        while (res->next()) {
            std::cout << std::left << std::setw(5) << res->getInt("subcategoryid")
                << std::setw(40) << res->getString("subcategoryname").substr(0,39)
                << std::endl;
            count ++;
        }
        std::cout << std::string(70, '=') << std::endl;
    } else {
        while (res->next()) {
            std::cout << "(" << res->getInt("subcategoryid") << ") "
                      << res->getString("subcategoryname") << std::endl;
            count++;
        }
    }
}

void listTasks(Database& db, int activityId, bool showAll) {
    std::string query = R"(
        SELECT t.taskid, t.taskname, t.taskdescription,
               t.departmentid, d.departmentname
        FROM ActivityTasks t
        LEFT JOIN Departments d ON t.departmentid = d.departmentid
    )";

    std::unique_ptr<sql::ResultSet> res;
    if (activityId > 0) {
        query += " WHERE t.activityid = ? AND t.parenttaskid IS NULL";
        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, activityId);
        res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
    } else {
        res = db.executeQuery(query);
    }

    int count = 0;
    std::cout << "Tasks:" << std::endl;
    if (showAll) {
        std::cout << std::string(127, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(40) << "TaskName"
                  << std::setw(40) << "TaskDescription"
                  << std::setw(37) << "DepartmentId"
                  << std::endl;
        std::cout << std::string(127, '-') << std::endl;
        while (res->next()) {
            std::string desc = res->isNull("taskdescription") ? "" : res->getString("taskdescription");
            std::string dept = res->isNull("departmentname") ? "" :
                std::to_string(res->getInt("departmentid")) + " (" + res->getString("departmentname") + ")";
            std::cout << std::left << std::setw(5) << res->getInt("taskid")
                      << std::setw(40) << res->getString("taskname").substr(0, 39)
                      << std::setw(40) << desc.substr(0, 39)
                      << std::setw(37) << dept.substr(0, 36)
                      << std::endl;
            count++;
        }
        std::cout << std::string(127, '=') << std::endl;
    } else {
        while (res->next()) {
            std::cout << "(" << res->getInt("taskid") << ") "
                      << res->getString("taskname") << std::endl;
            count++;
        }
    }
}

void listSubTasks(Database& db, int taskId, bool showAll) {
    std::string query = R"(
        SELECT t.taskid, t.taskname, t.taskdescription,
               t.departmentid, d.departmentname
        FROM ActivityTasks t
        LEFT JOIN Departments d ON t.departmentid = d.departmentid
        WHERE t.parenttaskid = ?
    )";

    auto stmt = db.prepareStatement(query);
    stmt->setInt(1, taskId);
    auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

    int count = 0;
    std::cout << "Tasks:" << std::endl;
    if (showAll) {
        std::cout << std::string(127, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(40) << "TaskName"
                  << std::setw(40) << "TaskDescription"
                  << std::setw(37) << "DepartmentId"
                  << std::endl;
        std::cout << std::string(127, '-') << std::endl;
        while (res->next()) {
            std::string desc = res->isNull("taskdescription") ? "" : res->getString("taskdescription");
            std::string dept = res->isNull("departmentname") ? "" :
                std::to_string(res->getInt("departmentid")) + " (" + res->getString("departmentname") + ")";
            std::cout << std::left << std::setw(5) << res->getInt("taskid")
                      << std::setw(40) << res->getString("taskname").substr(0, 39)
                      << std::setw(40) << desc.substr(0, 39)
                      << std::setw(37) << dept.substr(0, 36)
                      << std::endl;
            count++;
        }
        std::cout << std::string(127, '=') << std::endl;
    } else {
        while (res->next()) {
            std::cout << "(" << res->getInt("taskid") << ") "
                      << res->getString("taskname") << std::endl;
            count++;
        }
    }
}

void listProjects(Database& db, bool showAll){
    std::string query = R"(
        SELECT p.projectid, p.projectname, p.projectno
        FROM Projects p
    )";
    auto res = db.executeQuery(query);

    int count = 0;
    std::cout << "Projects:" << std::endl;
    if (showAll) {
        std::cout << std::string(70,'=') <<'\n';
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(15) << "ProjectNo"
                  << std::setw(45) << "ProjectName"
                  << std::endl;
        std::cout << std::string(70,'-') << std::endl;

        while (res->next()) {
            std::cout << std::left << std::setw(5) << res->getInt("projectid")
                << std::setw(15) << res->getString("projectno").substr(0, 14)
                << std::setw(45) << res->getString("projectname").substr(0, 44)
                << std::endl;
            count ++;
        }
        std::cout << std::string(70, '=') << std::endl;
    } else {
        while (res->next()) {
            std::cout << "(" << res->getInt("projectid") << ") "
                      << res->getString("projectname") << std::endl;
            count++;
        }
    }
}



void listEmployees(Database& db, int departmentId, bool showAll) {
    std::string query = R"(
        SELECT employeeid, firstname, lastname, email, admin
        FROM Employees
    )";

    std::unique_ptr<sql::ResultSet> res;
    if (departmentId > 0) {
        query += " WHERE departmentid = ? ORDER BY employeeid";
        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, departmentId);
        res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
    } else {
        query += " ORDER BY employeeid";
        res = db.executeQuery(query);
    }

    int count = 0;
    std::cout << "Employees:" << std::endl;
    if (showAll) {
        std::cout << std::string(100, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(20) << "FirstName"
                  << std::setw(20) << "LastName"
                  << std::setw(40) << "Email"
                  << std::setw(10) << "Admin"
                  << std::endl;
        std::cout << std::string(100, '-') << std::endl;

        while (res->next()) {
            std::string adminStr = res->getBoolean("admin") ? "Yes" : "No";
            std::cout << std::left << std::setw(5) << res->getInt("employeeid")
                      << std::setw(20) << res->getString("firstname").substr(0, 19)
                      << std::setw(20) << res->getString("lastname").substr(0, 19)
                      << std::setw(40) << res->getString("email").substr(0, 39)
                      << std::setw(10) << adminStr
                      << std::endl;
            count++;
        }
        std::cout << std::string(100, '=') << std::endl;
    } else {
        while (res->next()) {
            bool isAdmin = res->getBoolean("admin");
            std::cout << "(" << res->getInt("employeeid") << ") "
                      << res->getString("firstname") << " " << res->getString("lastname")
                      << (isAdmin ? " [Admin]" : "")
                      << std::endl;
            count++;
        }
    }
}
void listNotes(Database& db, int activityId, bool showAll){
    std::string query = R"(
        SELECT n.noteid, n.note, n.notedate,
               n.userid,
               CONCAT(e.firstname, ' ', e.lastname) as username
        FROM ActivityNotes n
        JOIN Employees e ON n.userid = e.employeeid
        WHERE n.activityid = ?
        ORDER BY n.notedate DESC
    )";

    auto stmt = db.prepareStatement(query);
    stmt->setInt(1, activityId);
    auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

    int count = 0;
    std::cout << "Activity Notes:" << std::endl;
    if (showAll) {
        std::cout << std::string(127, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(15) << "NoteDate"
                  << std::setw(32) << "UserId"
                  << std::setw(75) << "Note"
                  << std::endl;
        std::cout << std::string(127, '-') << std::endl;

        while (res->next()) {
            std::string user = std::to_string(res->getInt("userid")) + " (" + res->getString("username") + ")";
            std::cout << std::left << std::setw(5) << res->getInt("noteid")
                      << std::setw(15) << res->getString("notedate")
                      << std::setw(32) << user.substr(0, 31)
                      << std::setw(75) << res->getString("note").substr(0, 74)
                      << std::endl;
            count++;
        }
        std::cout << std::string(127, '=') << std::endl;
    } else {
        while (res->next()) {
            std::string notePreview = res->getString("note");
            if (notePreview.length() > 50) {
                notePreview = notePreview.substr(0, 47) + "...";
            }
            std::cout << "(" << res->getInt("noteid") << ") " << notePreview << std::endl;
            count++;
        }
    }
}

void listArtefactTypes(Database& db, int projectId, bool showAll){
    std::string query = R"(
        SELECT at.artefacttypeid, at.artefactname, at.artefactdescription
        FROM ArtefactTypes at
        WHERE at.projectid = ?
        ORDER BY at.artefacttypeid
    )";

    auto stmt = db.prepareStatement(query);
    stmt->setInt(1, projectId);
    auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

    int count = 0;
    std::cout << "Artefact Types:" << std::endl;
    if (showAll) {
        std::cout << std::string(120, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(40) << "ArtefactName"
                  << std::setw(75) << "ArtefactDescription"
                  << std::endl;
        std::cout << std::string(120, '-') << std::endl;

        while (res->next()) {
            std::cout << std::left << std::setw(5) << res->getInt("artefacttypeid")
                      << std::setw(40) << res->getString("artefactname").substr(0, 39)
                      << std::setw(75) << res->getString("artefactdescription").substr(0, 74)
                      << std::endl;
            count++;
        }
        std::cout << std::string(120, '=') << std::endl;
    } else {
        while (res->next()) {
            std::cout << "(" << res->getInt("artefacttypeid") << ") "
                      << res->getString("artefactname") << std::endl;
            count++;
        }
    }
}

void listArtefacts(Database& db, int artefactTypeId, bool showAll){
    std::string query = R"(
        SELECT a.artefactid, a.artefacttitle, a.artefactownerid,
               CONCAT(e.firstname, ' ', e.lastname) as ownername
        FROM Artefacts a
        LEFT JOIN Employees e ON a.artefactownerid = e.employeeid
        WHERE a.artefacttypeid = ?
        ORDER BY a.artefactid
    )";

    auto stmt = db.prepareStatement(query);
    stmt->setInt(1, artefactTypeId);
    auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

    int count = 0;
    std::cout << "Artefacts:" << std::endl;
    if (showAll) {
        std::cout << std::string(97, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(50) << "ArtefactTitle"
                  << std::setw(42) << "ArtefactOwnerId"
                  << std::endl;
        std::cout << std::string(97, '-') << std::endl;

        while (res->next()) {
            std::string owner = res->isNull("ownername") ? "" :
                std::to_string(res->getInt("artefactownerid")) + " (" + res->getString("ownername") + ")";
            std::cout << std::left << std::setw(5) << res->getInt("artefactid")
                      << std::setw(50) << res->getString("artefacttitle").substr(0, 49)
                      << std::setw(42) << owner.substr(0, 41)
                      << std::endl;
            count++;
        }
        std::cout << std::string(97, '=') << std::endl;
    } else {
        while (res->next()) {
            std::cout << "(" << res->getInt("artefactid") << ") "
                      << res->getString("artefacttitle") << std::endl;
            count++;
        }
    }
}

void listFields(Database& db, int artefactTypeId, bool showAll){
    std::string query = R"(
        SELECT adf.artefactdatafieldid, adf.fieldtitle, adf.valuetype,
               adf.maximumlength, adf.maximumvalue, adf.minimumvalue
        FROM ArtefactDataFields adf
        WHERE adf.artefacttypeid = ?
        ORDER BY adf.artefactdatafieldid
    )";

    auto stmt = db.prepareStatement(query);
    stmt->setInt(1, artefactTypeId);
    auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

    int count = 0;
    std::cout << "Artefact Fields:" << std::endl;
    if (showAll) {
        std::cout << std::string(120, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(40) << "FieldTitle"
                  << std::setw(15) << "ValueType"
                  << std::setw(20) << "MaximumLength"
                  << std::setw(20) << "MinimumValue"
                  << std::setw(20) << "MaximumValue"
                  << std::endl;
        std::cout << std::string(120, '-') << std::endl;

        while (res->next()) {
            int valueType = res->getInt("valuetype");
            std::string typeStr = (valueType == 0) ? "Text" : "Number";

            std::string maxLength = res->isNull("maximumlength") ? "N/A" : std::to_string(res->getInt("maximumlength"));
            std::string minValue = res->isNull("minimumvalue") ? "N/A" : std::to_string(res->getInt("minimumvalue"));
            std::string maxValue = res->isNull("maximumvalue") ? "N/A" : std::to_string(res->getInt("maximumvalue"));

            std::cout << std::left << std::setw(5) << res->getInt("artefactdatafieldid")
                      << std::setw(40) << res->getString("fieldtitle").substr(0, 39)
                      << std::setw(15) << typeStr
                      << std::setw(20) << maxLength
                      << std::setw(20) << minValue
                      << std::setw(20) << maxValue
                      << std::endl;
            count++;
        }
        std::cout << std::string(120, '=') << std::endl;
    } else {
        while (res->next()) {
            std::cout << "(" << res->getInt("artefactdatafieldid") << ") "
                      << res->getString("fieldtitle") << std::endl;
            count++;
        }
    }
}

void listMilestones(Database& db, int projectId, bool showAll){
    std::string query = R"(
        SELECT m.milestoneid, m.milestonename
        FROM Milestones m
        WHERE m.projectid = ?
        ORDER BY m.milestoneid
    )";

    auto stmt = db.prepareStatement(query);
    stmt->setInt(1, projectId);
    auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

    int count = 0;
    std::cout << "Milestones:" << std::endl;
    if (showAll) {
        std::cout << std::string(70, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(60) << "MilestoneName"
                  << std::endl;
        std::cout << std::string(70, '-') << std::endl;

        while (res->next()) {
            std::cout << std::left << std::setw(5) << res->getInt("milestoneid")
                      << std::setw(60) << res->getString("milestonename").substr(0, 59)
                      << std::endl;
            count++;
        }
        std::cout << std::string(70, '=') << std::endl;
    } else {
        while (res->next()) {
            std::cout << "(" << res->getInt("milestoneid") << ") "
                      << res->getString("milestonename") << std::endl;
            count++;
        }
    }
}

void listMilestoneSteps(Database& db, int milestoneId, bool showAll){
    std::string query = R"(
        SELECT ms.milestonestepid, ms.milestonestepname, ms.progressratio
        FROM MilestoneSteps ms
        WHERE ms.milestoneid = ?
        ORDER BY ms.progressratio, ms.milestonestepid
    )";

    auto stmt = db.prepareStatement(query);
    stmt->setInt(1, milestoneId);
    auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

    int count = 0;
    std::cout << "Milestone Steps:" << std::endl;
    if (showAll) {
        std::cout << std::string(90, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(50) << "MilestoneStepName"
                  << std::setw(20) << "ProgressRatio"
                  << std::endl;
        std::cout << std::string(90, '-') << std::endl;

        while (res->next()) {
            std::cout << std::left << std::setw(5) << res->getInt("milestonestepid")
                      << std::setw(50) << res->getString("milestonestepname").substr(0, 49)
                      << std::setw(20) << res->getDouble("progressratio")
                      << std::endl;
            count++;
        }
        std::cout << std::string(90, '=') << std::endl;
    } else {
        while (res->next()) {
            std::cout << "(" << res->getInt("milestonestepid") << ") "
                      << res->getString("milestonestepname") << std::endl;
            count++;
        }
    }
}

void listDepartments(Database& db, bool showAll) {
    std::string query = R"(
        SELECT departmentid, departmentname
        FROM Departments
        ORDER BY departmentid
    )";

    auto res = db.executeQuery(query);

    int count = 0;
    std::cout << "Departments:" << std::endl;
    if (showAll) {
        std::cout << std::string(70, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(60) << "DepartmentName"
                  << std::endl;
        std::cout << std::string(70, '-') << std::endl;

        while (res->next()) {
            std::cout << std::left << std::setw(5) << res->getInt("departmentid")
                      << std::setw(60) << res->getString("departmentname").substr(0, 59)
                      << std::endl;
            count++;
        }
        std::cout << std::string(70, '=') << std::endl;
    } else {
        while (res->next()) {
            std::cout << "(" << res->getInt("departmentid") << ") "
                      << res->getString("departmentname") << std::endl;
            count++;
        }
    }
}

void listTeamMembers(Database& db, int projectId, bool showAll) {
    std::string query = R"(
        SELECT ptm.teammemberid, ptm.role,
               ptm.employeeid,
               CONCAT(e.firstname, ' ', e.lastname) as name
        FROM ProjectTeamMembers ptm
        JOIN Employees e ON ptm.employeeid = e.employeeid
        WHERE ptm.projectid = ?
        ORDER BY ptm.teammemberid
    )";

    auto stmt = db.prepareStatement(query);
    stmt->setInt(1, projectId);
    auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

    int count = 0;
    std::cout << "Team Members:" << std::endl;
    if (showAll) {
        std::cout << std::string(67, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(37) << "EmployeeId"
                  << std::setw(25) << "Role"
                  << std::endl;
        std::cout << std::string(67, '-') << std::endl;

        while (res->next()) {
            int role = res->isNull("role") ? 0 : res->getInt("role");
            std::string roleDesc = (role == 0) ? "Member" : (role == 1) ? "Lead" : "Manager";
            std::string roleStr = std::to_string(role) + " (" + roleDesc + ")";
            std::string nameStr = std::to_string(res->getInt("employeeid")) + " (" + res->getString("name") + ")";
            std::cout << std::left << std::setw(5) << res->getInt("teammemberid")
                      << std::setw(37) << nameStr.substr(0, 36)
                      << std::setw(25) << roleStr
                      << std::endl;
            count++;
        }
        std::cout << std::string(67, '=') << std::endl;
    } else {
        while (res->next()) {
            std::cout << "(" << res->getInt("teammemberid") << ") "
                      << res->getString("name") << std::endl;
            count++;
        }
    }
}

void listAssignments(Database& db, int taskId, bool showAll) {
    std::string query = R"(
        SELECT ata.assignmentid, ata.assigneddate, ata.closedate,
               ata.userid,
               CONCAT(e.firstname, ' ', e.lastname) as name
        FROM ActivityTaskAssignments ata
        JOIN Employees e ON ata.userid = e.employeeid
        WHERE ata.taskid = ?
        ORDER BY ata.assignmentid
    )";

    auto stmt = db.prepareStatement(query);
    stmt->setInt(1, taskId);
    auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

    int count = 0;
    std::cout << "Task Assignments:" << std::endl;
    if (showAll) {
        std::cout << std::string(107, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(37) << "UserId"
                  << std::setw(15) << "AssignedDate"
                  << std::setw(15) << "CloseDate"
                  << std::endl;
        std::cout << std::string(107, '-') << std::endl;

        while (res->next()) {
            std::string assignedDate = res->isNull("assigneddate") ? "" : res->getString("assigneddate");
            std::string closeDate = res->isNull("closedate") ? "" : res->getString("closedate");
            std::string nameStr = std::to_string(res->getInt("userid")) + " (" + res->getString("name") + ")";
            std::cout << std::left << std::setw(5) << res->getInt("assignmentid")
                      << std::setw(37) << nameStr.substr(0, 36)
                      << std::setw(15) << assignedDate
                      << std::setw(15) << closeDate
                      << std::endl;
            count++;
        }
        std::cout << std::string(107, '=') << std::endl;
    } else {
        while (res->next()) {
            std::cout << "(" << res->getInt("assignmentid") << ") "
                      << res->getString("name") << std::endl;
            count++;
        }
    }
}

void listHours(Database& db, int assignmentId, bool showAll) {
    std::string query = R"(
        SELECT bookingid, bookeddate, hours
        FROM Hours
        WHERE assignmentid = ?
        ORDER BY bookeddate DESC
    )";

    auto stmt = db.prepareStatement(query);
    stmt->setInt(1, assignmentId);
    auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

    int count = 0;
    std::cout << "Time Booking:" << std::endl;
    if (showAll) {
        std::cout << std::string(50, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(20) << "BookedDate"
                  << std::setw(10) << "Hours"
                  << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        while (res->next()) {
            std::cout << std::left << std::setw(5) << res->getInt("bookingid")
                      << std::setw(20) << res->getString("bookeddate")
                      << std::setw(10) << res->getInt("hours")
                      << std::endl;
            count++;
        }
        std::cout << std::string(50, '=') << std::endl;
    } else {
        while (res->next()) {
            std::cout << "(" << res->getInt("bookingid") << ") "
                      << res->getString("bookeddate") << " - "
                      << res->getInt("hours") << "h" << std::endl;
            count++;
        }
    }
}

void listArtefactDataItems(Database& db, int artefactId, bool showAll) {
    std::string query = R"(
        SELECT ad.artefactdataid, adf.fieldtitle, ad.value
        FROM ArtefactData ad
        JOIN ArtefactDataFields adf ON ad.artefactdatafieldid = adf.artefactdatafieldid
        WHERE ad.artefactid = ?
        ORDER BY ad.artefactdataid
    )";

    auto stmt = db.prepareStatement(query);
    stmt->setInt(1, artefactId);
    auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

    int count = 0;
    std::cout << "Artefact Data:" << std::endl;
    if (showAll) {
        std::cout << std::string(120, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(40) << "FieldTitle"
                  << std::setw(70) << "Value"
                  << std::endl;
        std::cout << std::string(120, '-') << std::endl;

        while (res->next()) {
            std::string value = res->isNull("value") ? "" : res->getString("value");
            std::cout << std::left << std::setw(5) << res->getInt("artefactdataid")
                      << std::setw(40) << res->getString("fieldtitle").substr(0, 39)
                      << std::setw(70) << value.substr(0, 69)
                      << std::endl;
            count++;
        }
        std::cout << std::string(120, '=') << std::endl;
    } else {
        while (res->next()) {
            std::cout << "(" << res->getInt("artefactdataid") << ") "
                      << res->getString("fieldtitle") << std::endl;
            count++;
        }
    }
}

void listArtefactToActivityLinks(Database& db, int artefactId, bool showAll) {
    std::string query = R"(
        SELECT aal.artefactlinkid, aal.activityid, a.activityname, aal.ratio
        FROM ArtefactToActivityLink aal
        JOIN Activities a ON aal.activityid = a.activityid
        WHERE aal.artefactid = ?
        ORDER BY aal.artefactlinkid
    )";

    auto stmt = db.prepareStatement(query);
    stmt->setInt(1, artefactId);
    auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

    int count = 0;
    std::cout << "Activities Associated:" << std::endl;
    if (showAll) {
        std::cout << std::string(97, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(67) << "ActivityId"
                  << std::setw(15) << "Ratio"
                  << std::endl;
        std::cout << std::string(97, '-') << std::endl;

        while (res->next()) {
            double ratio = res->isNull("ratio") ? 0.0 : res->getDouble("ratio");
            std::string activityStr = std::to_string(res->getInt("activityid")) + " (" + res->getString("activityname") + ")";
            std::cout << std::left << std::setw(5) << res->getInt("artefactlinkid")
                      << std::setw(67) << activityStr.substr(0, 66)
                      << std::setw(15) << ratio
                      << std::endl;
            count++;
        }
        std::cout << std::string(97, '=') << std::endl;
    } else {
        while (res->next()) {
            std::cout << "(" << res->getInt("artefactlinkid") << ") "
                      << res->getString("activityname") << std::endl;
            count++;
        }
    }
}

void listMilestoneToArtefactLinks(Database& db, int artefactLinkId, bool showAll) {
    std::string query = R"(
        SELECT mal.milestonelinkid, mal.milestonestepid, ms.milestonestepname,
               mal.completiondate, mal.completedbyid,
               CONCAT(e.firstname, ' ', e.lastname) as completedby
        FROM MilestoneToArtefactLink mal
        JOIN MilestoneSteps ms ON mal.milestonestepid = ms.milestonestepid
        LEFT JOIN Employees e ON mal.completedbyid = e.employeeid
        WHERE mal.artefactlinkid = ?
        ORDER BY mal.milestonelinkid
    )";

    auto stmt = db.prepareStatement(query);
    stmt->setInt(1, artefactLinkId);
    auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

    int count = 0;
    std::cout << "Milestones Associated:" << std::endl;
    if (showAll) {
        std::cout << std::string(134, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(47) << "MilestoneStepId"
                  << std::setw(15) << "CompletionDate"
                  << std::setw(37) << "CompletedById"
                  << std::endl;
        std::cout << std::string(134, '-') << std::endl;

        while (res->next()) {
            std::string completionDate = res->isNull("completiondate") ? "" : res->getString("completiondate");
            std::string stepStr = std::to_string(res->getInt("milestonestepid")) + " (" + res->getString("milestonestepname") + ")";
            std::string completedBy = res->isNull("completedby") ? "" :
                std::to_string(res->getInt("completedbyid")) + " (" + res->getString("completedby") + ")";
            std::cout << std::left << std::setw(5) << res->getInt("milestonelinkid")
                      << std::setw(47) << stepStr.substr(0, 46)
                      << std::setw(15) << completionDate
                      << std::setw(37) << completedBy.substr(0, 36)
                      << std::endl;
            count++;
        }
        std::cout << std::string(134, '=') << std::endl;
    } else {
        while (res->next()) {
            std::cout << "(" << res->getInt("milestonelinkid") << ") "
                      << res->getString("milestonestepname") << std::endl;
            count++;
        }
    }
}

} // namespace commands
