#include "importcsv.h"

#include "add.h"
#include "set.h"
#include "../exceptions/exceptions.h"
#include "../utils/csv_parser.h"
#include "../utils/lookup.h"
#include <iostream>

namespace commands {

void importcsv (Database& db, Path& pth, std::string filename, bool ignoreDuplicates) {

    try {
        int parentId = pth.current_id();
        std::vector<std::map<std::string,std::string>> CSVData;
        CSVData = utils::parseCSVFile(filename);

        auto importRows = [&](std::vector<std::map<std::string,std::string>> rows) {
            switch (pth.current_type()) {
            case Path::ROOT:
                throw NavigationError("Cannot add an item here.");
            case Path::ORGANIZATION_FOLDER: {
                DepartmentData dept(db, rows);
                addDepartment(db, dept);
                break;
            }
            case Path::DEPARTMENT: {
                EmployeeData emp(db, rows, parentId);
                addEmployee(db, emp);
                break;
            }
            case Path::PROJECTS_FOLDER: {
                ProjectData proj(db, rows);
                addProject(db, proj);
                break;
            }
            case Path::PROJECT:
                throw NavigationError("Cannot add an item here.");
            case Path::ACTIVITIES_FOLDER: {
                CategoryData cat(db, rows, parentId);
                addCategory(db, cat, parentId);
                break;
            }
            case Path::CATEGORY: {
                SubCategoryData subCat(db, rows, parentId);
                addSubcategory(db, subCat, parentId);
                break;
            }
            case Path::SUBCATEGORY: {
                ActivityData act(db, rows, parentId);
                addActivity(db, act, parentId, pth.getIdByType(Path::PROJECT));
                break;
            }
            case Path::ACTIVITY:
                throw NavigationError("Cannot add an item here.");
            case Path::TASKS_FOLDER: {
                if (pth.getIdByType(Path::TASK) > 0) {
                    TaskData tsk(db, rows, pth.getIdByType(Path::ACTIVITY), parentId);
                    addTask(db, tsk, pth.getIdByType(Path::ACTIVITY), parentId);
                } else {
                    TaskData tsk(db, rows, parentId, 0);
                    addTask(db, tsk, parentId, 0);
                }
                break;
            }
            case Path::TASK:
                throw NavigationError("Cannot add an item here. Navigate to Tasks folder first.");
            case Path::NOTES_FOLDER: {
                NoteData noe(db, rows, parentId);
                addNote(db, noe, pth.getIdByType(Path::PROJECT));
                break;
            }
            case Path::ARTEFACT_TYPES_FOLDER: {
                ArtefactTypeData artTyp(db, rows, parentId);
                addArtefactType(db, artTyp);
                break;
            }
            case Path::ARTEFACT_TYPE:
                throw NavigationError("Cannot add an item here.");
            case Path::ARTEFACTS_FOLDER: {
                ArtefactData art(db, rows, parentId);
                addArtefact(db, art, pth.getIdByType(Path::PROJECT));
                break;
            }
            case Path::FIELDS_FOLDER: {
                FieldData fie(db, rows, parentId);
                addField(db, fie);
                break;
            }
            case Path::MILESTONES_FOLDER: {
                MilestoneData mil(db, rows, parentId);
                addMilestone(db, mil);
                break;
            }
            case Path::MILESTONE: {
                MilestoneStepData milStp(db, rows, parentId);
                addMilestoneStep(db, milStp);
                break;
            }
            case Path::TEAM_FOLDER: {
                ProjectTeamMemberData member(db, rows, pth.getIdByType(Path::PROJECT));
                addTeamMember(db, member);
                break;
            }
            case Path::ASSIGNMENTS_FOLDER: {
                AssignmentData assignment(db, rows, parentId);
                addAssignment(db, assignment, pth.getIdByType(Path::PROJECT));
                break;
            }
            case Path::ASSIGNMENT: {
                HoursData hrs(db, rows, parentId);
                addHours(db, hrs);
                break;
            }
            case Path::DATA_FOLDER: {
                ArtefactDataItemData dataItem(db, rows, pth.getIdByType(Path::ARTEFACT), pth.getIdByType(Path::ARTEFACT_TYPE));
                addArtefactDataItem(db, dataItem);
                break;
            }
            case Path::ASSOCIATIONS_FOLDER: {
                ArtefactToActivityLinkData link(db, rows, pth.getIdByType(Path::ARTEFACT), pth.getIdByType(Path::PROJECT));
                addArtefactToActivityLink(db, link);
                break;
            }
            case Path::ARTEFACT_TO_ACTIVITY_LINK: {
                MilestoneToArtefactLinkData link(db, rows, parentId, 0 /*TODO milestoneId would be needed for scoped lookup of milestone step names*/);
                addMilestoneToArtefactLink(db, link, pth.getIdByType(Path::PROJECT));
                break;
            }
            }
        };

        if (!ignoreDuplicates) {
            importRows(CSVData);
        } else {
            int skipped = 0;
            for (const auto& row : CSVData) {
                try {
                    importRows({row});
                } catch (const DuplicateEntryError& e) {
                    std::cout << "Skipping duplicate: " << e.what() << std::endl;
                    skipped++;
                }
            }
            if (skipped > 0) {
                std::cout << "Skipped " << skipped << " duplicate" << (skipped > 1 ? "s" : "") << "." << std::endl;
            }
        }
    } catch (const DataError& e) {
        throw;
    }
}

void importcsvRecursive(Database& db, Path& pth, std::string filename) {
    int parentId = pth.current_id();
    std::vector<std::map<std::string,std::string>> CSVData;
    CSVData = utils::parseCSVFile(filename);

    switch (pth.current_type()) {
    case Path::ORGANIZATION_FOLDER:
        addDepartmentsRecursive(db, CSVData);
        break;
    case Path::DEPARTMENT:
        addEmployeesRecursive(db, CSVData, parentId);
        break;
    case Path::PROJECTS_FOLDER:
        addProjectsRecursive(db, CSVData);
        break;
    case Path::ACTIVITIES_FOLDER:
        addCategoriesRecursive(db, CSVData, parentId);
        break;
    case Path::CATEGORY:
        addSubcategoriesRecursive(db, CSVData, parentId, pth.getIdByType(Path::PROJECT));
        break;
    case Path::SUBCATEGORY:
        addActivitiesRecursive(db, CSVData, parentId, pth.getIdByType(Path::PROJECT));
        break;
    case Path::TASKS_FOLDER: {
        int actId = pth.getIdByType(Path::ACTIVITY);
        int parentTaskId = (pth.getIdByType(Path::TASK) > 0) ? parentId : 0;
        addTasksRecursive(db, CSVData, actId, parentTaskId, pth.getIdByType(Path::PROJECT));
        break;
    }
    case Path::NOTES_FOLDER:
        addNotesRecursive(db, CSVData, parentId, pth.getIdByType(Path::PROJECT));
        break;
    case Path::ARTEFACT_TYPES_FOLDER:
        addArtefactTypesRecursive(db, CSVData, parentId);
        break;
    case Path::ARTEFACTS_FOLDER:
        addArtefactsRecursive(db, CSVData, parentId, pth.getIdByType(Path::PROJECT));
        break;
    case Path::FIELDS_FOLDER:
        addFieldsRecursive(db, CSVData, parentId);
        break;
    case Path::MILESTONES_FOLDER:
        addMilestonesRecursive(db, CSVData, parentId);
        break;
    case Path::MILESTONE:
        addMilestoneStepsRecursive(db, CSVData, parentId);
        break;
    case Path::TEAM_FOLDER:
        addTeamMembersRecursive(db, CSVData, parentId);
        break;
    case Path::ASSIGNMENTS_FOLDER:
        addAssignmentsRecursive(db, CSVData, parentId, pth.getIdByType(Path::PROJECT));
        break;
    case Path::ASSIGNMENT:
        addHoursRecursive(db, CSVData, parentId);
        break;
    case Path::DATA_FOLDER:
        addArtefactDataItemsRecursive(db, CSVData, parentId, pth.getIdByType(Path::ARTEFACT_TYPE));
        break;
    case Path::ASSOCIATIONS_FOLDER:
        addArtefactToActivityLinksRecursive(db, CSVData, parentId, pth.getIdByType(Path::PROJECT));
        break;
    case Path::ARTEFACT_TO_ACTIVITY_LINK:
        addMilestoneToArtefactLinksRecursive(db, CSVData, parentId, pth.getIdByType(Path::PROJECT));
        break;
    default:
        throw NavigationError("Recursive import is not supported at this location.");
    }
}

void addDepartmentsRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows) {
    if (!rows[0].count("departmentname") || rows[0].at("departmentname").empty()) return;

    std::vector<std::map<std::string,std::string>> rowSlice;
    int departmentId;
    for (const auto& row : rows) {
        try {
            rowSlice = std::vector<std::map<std::string,std::string>>{row};
            DepartmentData newDept(db, rowSlice, true);
            std::string departmentName = newDept.getName();
            auto stmt = db.prepareStatement("SELECT departmentid FROM Departments WHERE departmentname = ?");
            stmt->setString(1, departmentName);
            auto res = stmt->executeQuery();
            if (!res->next()) {
                departmentId = addDepartment(db, newDept);
            } else {
                departmentId = res->getInt("departmentid");
                DepartmentData existDept(departmentId, &db);
                if (!row.at("departmentname").empty()) existDept.setValue("departmentname", row.at("departmentname"));
                setDepartmentData(db, existDept, departmentId);
            }
        } catch (DataError& e) {
            std::cout << "Warning: " << e.what() << " Skipping item." << std::endl;
            continue;
        }
        addEmployeesRecursive(db, rowSlice, departmentId);
    }
}

void addEmployeesRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int departmentId) {
    if (!rows[0].count("email") || rows[0].at("email").empty()) return;

    std::vector<std::map<std::string,std::string>> rowSlice;
    int employeeId;
    for (const auto& row : rows) {
        try {
            rowSlice = std::vector<std::map<std::string,std::string>>{row};
            EmployeeData newEmp(db, rowSlice, departmentId, true);
            std::string email = newEmp.getEmail();
            auto stmt = db.prepareStatement("SELECT employeeid FROM Employees WHERE email = ? AND departmentid = ?");
            stmt->setString(1, email);
            stmt->setInt(2, departmentId);
            auto res = stmt->executeQuery();
            if (!res->next()) {
                employeeId = addEmployee(db, newEmp);
            } else {
                employeeId = res->getInt("employeeid");
                EmployeeData existEmp(employeeId, &db);
                if (row.count("firstname") && !row.at("firstname").empty()) existEmp.setValue("firstname", row.at("firstname"));
                if (row.count("lastname") && !row.at("lastname").empty()) existEmp.setValue("lastname", row.at("lastname"));
                if (!row.at("email").empty()) existEmp.setValue("email", row.at("email"));
                setEmployeeData(db, existEmp, employeeId);
            }
        } catch (DataError& e) {
            std::cout << "Warning: " << e.what() << " Skipping item." << std::endl;
            continue;
        }
        (void)employeeId;
    }
}
void addProjectsRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows) {
    if (!rows[0].count("projectno") || rows[0].at("projectno").empty()) return;

    std::vector<std::map<std::string,std::string>> rowSlice;
    int projectId;
    for (const auto& row : rows) {
        try {
            rowSlice = std::vector<std::map<std::string,std::string>>{row};
            ProjectData newProj(db, rowSlice, true);
            std::string projectNo = newProj.getNumber();
            auto stmt = db.prepareStatement("SELECT projectid FROM Projects WHERE projectno = ?");
            stmt->setString(1, projectNo);
            auto res = stmt->executeQuery();
            if (!res->next()) {
                projectId = addProject(db, newProj);
            } else {
                projectId = res->getInt("projectid");
                ProjectData existProj(projectId, &db);
                if (!row.at("projectno").empty()) existProj.setValue("projectno", row.at("projectno"));
                if (row.count("projectname") && !row.at("projectname").empty()) existProj.setValue("projectname", row.at("projectname"));
                setProjectData(db, existProj, projectId);
            }
        } catch (DataError& e) {
            std::cout << "Warning: " << e.what() << " Skipping item." << std::endl;
            continue;
        }
        addCategoriesRecursive(db, rowSlice, projectId);
        addArtefactTypesRecursive(db, rowSlice, projectId);
        addMilestonesRecursive(db, rowSlice, projectId);
        addTeamMembersRecursive(db, rowSlice, projectId);
    }
}

void addCategoriesRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int projectId) {
    if (!rows[0].count("categoryname") || rows[0].at("categoryname").empty()) return;

    std::vector<std::map<std::string,std::string>> rowSlice;
    int categoryId;
    for (const auto& row : rows) {
        try {
            rowSlice = std::vector<std::map<std::string,std::string>>{row};
            CategoryData newCat(db, rowSlice, projectId, true);
            std::string categoryName = newCat.getName();
            auto stmt = db.prepareStatement("SELECT categoryid FROM ActivityCategories WHERE categoryname = ? AND projectid = ?");
            stmt->setString(1, categoryName);
            stmt->setInt(2, projectId);
            auto res = stmt->executeQuery();
            if (!res->next()) {
                categoryId = addCategory(db, newCat, projectId);
            } else {
                categoryId = res->getInt("categoryid");
                CategoryData existCat(categoryId, &db);
                if (!row.at("categoryname").empty()) existCat.setValue("categoryname", row.at("categoryname"));
                setCategoryData(db, existCat, categoryId);
            }
        } catch (DataError& e) {
            std::cout << "Warning: " << e.what() << " Skipping item." << std::endl;
            continue;
        }
        addSubcategoriesRecursive(db, rowSlice, categoryId, projectId);
    }
}

void addSubcategoriesRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int categoryId, int projectId) {
    if (!rows[0].count("subcategoryname") || rows[0].at("subcategoryname").empty()) return;

    std::vector<std::map<std::string,std::string>> rowSlice;
    int subcategoryId;
    for (const auto& row : rows) {
        try {
            rowSlice = std::vector<std::map<std::string,std::string>>{row};
            SubCategoryData newSubCat(db, rowSlice, categoryId, true);
            std::string subcategoryName = newSubCat.getName();
            auto stmt = db.prepareStatement("SELECT subcategoryid FROM ActivitySubcategories WHERE subcategoryname = ? AND categoryid = ?");
            stmt->setString(1, subcategoryName);
            stmt->setInt(2, categoryId);
            auto res = stmt->executeQuery();
            if (!res->next()) {
                subcategoryId = addSubcategory(db, newSubCat, categoryId);
            } else {
                subcategoryId = res->getInt("subcategoryid");
                SubCategoryData existSubCat(subcategoryId, &db);
                if (!row.at("subcategoryname").empty()) existSubCat.setValue("subcategoryname", row.at("subcategoryname"));
                setSubcategoryData(db, existSubCat, subcategoryId);
            }
        } catch (DataError& e) {
            std::cout << "Warning: " << e.what() << " Skipping item." << std::endl;
            continue;
        }
        addActivitiesRecursive(db, rowSlice, subcategoryId, projectId);
    }
}

void addActivitiesRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int subcategoryId, int projectId) {
    if (!rows[0].count("activityname") || rows[0].at("activityname").empty()) return;

    std::vector<std::map<std::string,std::string>> rowSlice;
    int activityId;
    for (const auto& row : rows) {
        try {
            rowSlice = std::vector<std::map<std::string,std::string>>{row};
            ActivityData newAct(db, rowSlice, subcategoryId, true);
            std::string activityName = newAct.getName();
            auto stmt = db.prepareStatement("SELECT activityid FROM Activities WHERE activityname = ? AND subcategoryid = ?");
            stmt->setString(1, activityName);
            stmt->setInt(2, subcategoryId);
            auto res = stmt->executeQuery();
            if (!res->next()) {
                activityId = addActivity(db, newAct, subcategoryId, 0);
            } else {
                activityId = res->getInt("activityid");
                ActivityData existAct(activityId, &db);
                if (!row.at("activityname").empty()) existAct.setValue("activityname", row.at("activityname"));
                if (row.count("activitydescription") && !row.at("activitydescription").empty()) existAct.setValue("activitydescription", row.at("activitydescription"));
                setActivityData(db, existAct, activityId);
            }
        } catch (DataError& e) {
            std::cout << "Warning: " << e.what() << " Skipping item." << std::endl;
            continue;
        }
        addTasksRecursive(db, rowSlice, activityId, 0, projectId);
        addNotesRecursive(db, rowSlice, activityId, projectId);
    }
}

void addTasksRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int activityId, int parentTaskId, int projectId) {
    if (!rows[0].count("taskname") || rows[0].at("taskname").empty()) return;

    std::vector<std::map<std::string,std::string>> rowSlice;
    int taskId;
    for (const auto& row : rows) {
        try {
            rowSlice = std::vector<std::map<std::string,std::string>>{row};
            int resolvedParentTaskId = parentTaskId;
            if (resolvedParentTaskId == 0) {
                if (row.count("parenttaskid") && !row.at("parenttaskid").empty()) {
                    resolvedParentTaskId = std::stoi(row.at("parenttaskid"));
                } else if (row.count("parenttaskname") && !row.at("parenttaskname").empty()) {
                    std::string activityName    = row.count("activityname")    ? row.at("activityname")    : "";
                    std::string subcategoryName = row.count("subcategoryname") ? row.at("subcategoryname") : "";
                    std::string categoryName    = row.count("categoryname")    ? row.at("categoryname")    : "";
                    utils::LookupResult lr = utils::lookupTaskByNames(db, row.at("parenttaskname"), projectId, activityName, subcategoryName, categoryName);
                    if (lr.found && !lr.multipleMatches) {
                        resolvedParentTaskId = lr.id;
                    } else if (lr.multipleMatches) {
                        throw ValidationError("Multiple tasks match parent task name '" + row.at("parenttaskname") + "'. Provide activityname, subcategoryname, or categoryname to disambiguate.");
                    } else {
                        throw ValidationError("Parent task not found: " + row.at("parenttaskname"));
                    }
                }
            }
            TaskData newTsk(db, rowSlice, activityId, resolvedParentTaskId, true);
            std::string taskName = newTsk.getName();
            std::unique_ptr<sql::PreparedStatement> stmt;
            if (resolvedParentTaskId > 0) {
                stmt = db.prepareStatement("SELECT taskid FROM ActivityTasks WHERE taskname = ? AND activityid = ? AND parenttaskid = ?");
                stmt->setString(1, taskName);
                stmt->setInt(2, activityId);
                stmt->setInt(3, resolvedParentTaskId);
            } else {
                stmt = db.prepareStatement("SELECT taskid FROM ActivityTasks WHERE taskname = ? AND activityid = ? AND parenttaskid IS NULL");
                stmt->setString(1, taskName);
                stmt->setInt(2, activityId);
            }
            auto res = stmt->executeQuery();
            if (!res->next()) {
                taskId = addTask(db, newTsk, activityId, resolvedParentTaskId);
            } else {
                taskId = res->getInt("taskid");
                TaskData existTsk(taskId, &db);
                if (!row.at("taskname").empty()) existTsk.setValue("taskname", row.at("taskname"));
                if (row.count("taskdescription") && !row.at("taskdescription").empty()) existTsk.setValue("taskdescription", row.at("taskdescription"));
                setTaskData(db, existTsk, taskId);
            }
        } catch (DataError& e) {
            std::cout << "Warning: " << e.what() << " Skipping item." << std::endl;
            continue;
        }
        addAssignmentsRecursive(db, rowSlice, taskId, projectId);

    }
}

void addAssignmentsRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int taskId, int projectId) {
    if (!rows[0].count("userid") || rows[0].at("userid").empty()) {
        if (!rows[0].count("username") || rows[0].at("username").empty()) return;
    }

    std::vector<std::map<std::string,std::string>> rowSlice;
    int assignmentId;
    for (const auto& row : rows) {
        try {
            rowSlice = std::vector<std::map<std::string,std::string>>{row};
            AssignmentData newAssignment(db, rowSlice, taskId, true);
            int userId = newAssignment.getUserId();
            auto stmt = db.prepareStatement("SELECT assignmentid FROM ActivityTaskAssignments WHERE userid = ? AND taskid = ?");
            stmt->setInt(1, userId);
            stmt->setInt(2, taskId);
            auto res = stmt->executeQuery();
            if (!res->next()) {
                assignmentId = addAssignment(db, newAssignment, projectId);
            } else {
                assignmentId = res->getInt("assignmentid");
                AssignmentData existAssignment(assignmentId, &db);
                if (row.count("assigneddate") && !row.at("assigneddate").empty()) existAssignment.setValue("assigneddate", row.at("assigneddate"));
                if (row.count("closedate") && !row.at("closedate").empty()) existAssignment.setValue("closedate", row.at("closedate"));
                setAssignmentData(db, existAssignment, assignmentId);
            }
        } catch (DataError& e) {
            std::cout << "Warning: " << e.what() << " Skipping item." << std::endl;
            continue;
        }
        addHoursRecursive(db, rowSlice, assignmentId);
    }
}

void addHoursRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int assignmentId) {
    if (!rows[0].count("bookeddate") || rows[0].at("bookeddate").empty()) return;
    if (!rows[0].count("hours") || rows[0].at("hours").empty()) return;

    std::vector<std::map<std::string,std::string>> rowSlice;
    int bookingId;
    for (const auto& row : rows) {
        try {
            rowSlice = std::vector<std::map<std::string,std::string>>{row};
            HoursData newHrs(db, rowSlice, assignmentId, true);
            std::string bookedDate = newHrs.getBookedDate();
            auto stmt = db.prepareStatement("SELECT bookingid FROM Hours WHERE bookeddate = ? AND assignmentid = ?");
            stmt->setString(1, bookedDate);
            stmt->setInt(2, assignmentId);
            auto res = stmt->executeQuery();
            if (!res->next()) {
                bookingId = addHours(db, newHrs);
            } else {
                bookingId = res->getInt("bookingid");
                HoursData existHrs(bookingId, &db);
                if (!row.at("bookeddate").empty()) existHrs.setValue("bookeddate", row.at("bookeddate"));
                if (row.count("hours") && !row.at("hours").empty()) existHrs.setValue("hours", row.at("hours"));
                setHoursData(db, existHrs, bookingId);
            }
        } catch (DataError& e) {
            std::cout << "Warning: " << e.what() << " Skipping item." << std::endl;
            continue;
        }
        (void)bookingId;
    }
}
void addNotesRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int activityId, int projectId) {
    if (!rows[0].count("note") || rows[0].at("note").empty()) return;

    std::vector<std::map<std::string,std::string>> rowSlice;
    int noteId;
    for (const auto& row : rows) {
        try {
            rowSlice = std::vector<std::map<std::string,std::string>>{row};
            NoteData newNote(db, rowSlice, activityId, true);
            std::string noteDate = newNote.getNoteDate();
            std::unique_ptr<sql::PreparedStatement> stmt;
            if (activityId > 0) {
                stmt = db.prepareStatement("SELECT noteid FROM ActivityNotes WHERE notedate = ? AND activityid = ?");
                stmt->setString(1, noteDate);
                stmt->setInt(2, activityId);
            } else {
                stmt = db.prepareStatement("SELECT noteid FROM ActivityNotes WHERE notedate = ? AND activityid IS NULL");
                stmt->setString(1, noteDate);
            }
            auto res = stmt->executeQuery();
            if (!res->next()) {
                noteId = addNote(db, newNote, projectId);
            } else {
                noteId = res->getInt("noteid");
                NoteData existNote(noteId, &db);
                if (!row.at("note").empty()) existNote.setValue("note", row.at("note"));
                setNoteData(db, existNote, noteId);
            }
        } catch (DataError& e) {
            std::cout << "Warning: " << e.what() << " Skipping item." << std::endl;
            continue;
        }
        (void)noteId;
    }
}

void addTeamMembersRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int projectId) {
    if (!rows[0].count("employeeid") || rows[0].at("employeeid").empty()) {
        if (!rows[0].count("employeename") || rows[0].at("employeename").empty()) return;
    }
    std::vector<std::map<std::string,std::string>> rowSlice;
    int teamMemberId;
    for (const auto& row : rows) {
        try {
            rowSlice = std::vector<std::map<std::string,std::string>>{row};
            ProjectTeamMemberData newMember(db, rowSlice, projectId, true);
            int employeeId = newMember.getEmployeeId();
            auto stmt = db.prepareStatement("SELECT teammemberid FROM ProjectTeamMembers WHERE employeeid = ? AND projectid = ?");
            stmt->setInt(1, employeeId);
            stmt->setInt(2, projectId);
            auto res = stmt->executeQuery();
            if (!res->next()) {
                teamMemberId = addTeamMember(db, newMember);
            } else {
                teamMemberId = res->getInt("teammemberid");
                ProjectTeamMemberData existMember(teamMemberId, &db);
                if (row.count("role") && !row.at("role").empty()) existMember.setValue("role", row.at("role"));
                setTeamMemberData(db, existMember, teamMemberId);
            }
        } catch (DataError& e) {
            std::cout << "Warning: " << e.what() << " Skipping item." << std::endl;
            continue;
        }
        (void)teamMemberId;
    }
}


void addArtefactTypesRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int projectId) {
    // Check if the required fields are present
    if (!rows[0].count("artefactname") || rows[0].at("artefactname").empty()) return;

    std::vector<std::map<std::string,std::string>> rowSlice;
    int artefactTypeId;
    std::string artefactName;
    for (const auto& row : rows) {
        try {
            // Try to build a data object from data
            rowSlice = std::vector<std::map<std::string,std::string>>{row};
            ArtefactTypeData newArtTyp(db, rowSlice, projectId, true);
            artefactName = newArtTyp.getName();
            // Check if the input data already exists
            auto stmt = db.prepareStatement("SELECT artefacttypeid FROM ArtefactTypes WHERE artefactname = ? AND projectid = ?");
            stmt->setString(1,artefactName);
            stmt->setInt(2,projectId);
            auto res = stmt->executeQuery();
            if (!res->next()) {
                //create a new item
                artefactTypeId = addArtefactType(db, newArtTyp);
            } else {
                // Update the existing item
                artefactTypeId = res->getInt("artefacttypeid");
                ArtefactTypeData existArtTyp(artefactTypeId, &db);
                if (!row.at("artefactname").empty()) existArtTyp.setValue("artefactname", row.at("artefactname"));
                if (row.count("artefactdescription") && !row.at("artefactdescription").empty()) existArtTyp.setValue("artefactdescription", row.at("artefactdescription"));
                setArtefactTypeData(db, existArtTyp, artefactTypeId);
            } 
        } catch (DataError& e) {
            std::cout<< "Warning: " <<  e.what() << " Skipping item." << std::endl;
            continue;
        }
        addArtefactsRecursive(db, rowSlice, artefactTypeId, projectId);
        addFieldsRecursive(db, rowSlice, artefactTypeId);
    }

}

void addArtefactsRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int artefactTypeId, int projectId) {
    // Check if the required fields are present
    if (!rows[0].count("artefacttitle") || rows[0].at("artefacttitle").empty()) return;

    std::vector<std::map<std::string,std::string>> rowSlice;
    int artefactId;
    std::string artefactTitle;
    int artefactOwnerId;
    for (const auto& row : rows) {
        try {
            // Try to build a data object from data
            rowSlice = std::vector<std::map<std::string,std::string>>{row};
            ArtefactData newArt(db, rowSlice, artefactTypeId, true);
            artefactTitle = newArt.getName();
            artefactOwnerId = newArt.getArtefactOwnerId();
            // Check if the input data already exists
            auto stmt = db.prepareStatement("SELECT artefactid FROM Artefacts WHERE artefacttitle = ? AND artefacttypeid = ?");
            stmt->setString(1,artefactTitle);
            stmt->setInt(2, artefactTypeId);
            auto res = stmt->executeQuery();
            if (!res->next()) {
                //create a new item
                artefactId = addArtefact(db, newArt);
            } else {
                // Update the existing item
                artefactId = res->getInt("artefactid");
                ArtefactData existArt(artefactId, &db);
                if (!row.at("artefacttitle").empty()) existArt.setValue("artefacttitle", row.at("artefacttitle"));
                existArt.setValue("artefactownerid", std::to_string(artefactOwnerId));
                setArtefactData(db, existArt, artefactId);
            } 
        } catch (DataError& e) {
            std::cout<< "Warning: " <<  e.what() << " Skipping item." << std::endl;
            continue;
        }
        addArtefactToActivityLinksRecursive(db, rowSlice, artefactId, projectId);
        addArtefactDataItemsRecursive (db, rowSlice, artefactId, artefactTypeId);
    }
}

void addArtefactToActivityLinksRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int artefactId, int projectId) {
    // Check if the required fields are present
    if (!rows[0].count("activityid") || rows[0].at("activityid").empty()) {
        if (!rows[0].count("activityname") || rows[0].at("activityname").empty()) return;
    }

    std::vector<std::map<std::string,std::string>> rowSlice;
    int artefactLinkId;
    int activityId;
    for (const auto& row : rows) {
        try {
            // Try to build a data object from data
            rowSlice = std::vector<std::map<std::string,std::string>>{row};
            ArtefactToActivityLinkData newArtLnk(db, rowSlice, artefactId, projectId, true);
            activityId = newArtLnk.getActivityId();
            // Check if the input data already exists
            auto stmt = db.prepareStatement("SELECT artefactlinkid FROM ArtefactToActivityLink WHERE artefactid = ? AND activityid = ?");
            stmt->setInt(1,artefactId);
            stmt->setInt(2,activityId);
            auto res = stmt->executeQuery();
            if (!res->next()) {
                //create a new item
                artefactLinkId = addArtefactToActivityLink(db, newArtLnk);
            } else {
                // Update the existing item
                artefactLinkId = res->getInt("artefactlinkid");
                ArtefactToActivityLinkData existArtLnk(artefactLinkId, &db);
                if (row.count("ratio") && !row.at("ratio").empty()) existArtLnk.setValue("ratio", row.at("ratio"));
                setArtefactToActivityLinkData(db, existArtLnk, artefactLinkId);
            } 
        } catch (DataError& e) {
            std::cout<< "Warning: " <<  e.what() << " Skipping item." << std::endl;
            continue;
        }
        addMilestoneToArtefactLinksRecursive(db, rowSlice,artefactLinkId, projectId);
    }
}

void addMilestoneToArtefactLinksRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int artefactLinkId,  int projectId) {
    // Check if the required fields are present
    if (!rows[0].count("milestonestepid") || rows[0].at("milestonestepid").empty()) {
        if (!rows[0].count("milestonestepname") || rows[0].at("milestonestepname").empty()) return;
    }

    std::vector<std::map<std::string,std::string>> rowSlice;
    int milestoneLinkId;
    for (const auto& row : rows) {
        try {
            // Try to build a data object from data
            rowSlice = std::vector<std::map<std::string,std::string>>{row};
            MilestoneToArtefactLinkData newMilLnk(db, rowSlice, artefactLinkId, projectId, true);
            int milestoneStepId = newMilLnk.getMilestoneStepId();
            int completedById = newMilLnk.getCompletedById();
            // Check if the input data already exists
            auto stmt = db.prepareStatement("SELECT milestonelinkid FROM MilestoneToArtefactLink WHERE artefactlinkid = ? AND milestonestepid = ?");
            stmt->setInt(1,artefactLinkId);
            stmt->setInt(2,milestoneStepId);
            auto res = stmt->executeQuery();
            if (!res->next()) {
                //create a new item
                milestoneLinkId = addMilestoneToArtefactLink(db, newMilLnk, projectId);
            } else {
                // Update the existing item
                milestoneLinkId = res->getInt("milestonelinkid");
                MilestoneToArtefactLinkData existMilLnk(milestoneLinkId, &db);
                existMilLnk.setValue("completedbyid", std::to_string(completedById));
                if (row.count("completiondate") && !row.at("completiondate").empty()) existMilLnk.setValue("completiondate", row.at("completiondate"));
                setMilestoneToArtefactLinkData(db, existMilLnk, milestoneLinkId);
            } 
        } catch (DataError& e) {
            std::cout<< "Warning: " <<  e.what() << " Skipping item." << std::endl;
            continue;
        }
        // Milestone to artefact links are the terminal node in the artefact association hierarchy
        (void)milestoneLinkId;
    }
}

void addFieldsRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int artefactTypeId) {
    // Check if the required fields are present
    if (!rows[0].count("fieldtitle") || rows[0].at("fieldtitle").empty()) return;
    if (!rows[0].count("valuetype") || rows[0].at("valuetype").empty()) return;

    std::vector<std::map<std::string,std::string>> rowSlice;
    int artefactDataFieldId;
    for (const auto& row : rows) {
        try {
            // Try to build a data object from data
            rowSlice = std::vector<std::map<std::string,std::string>>{row};
            FieldData newFld(db, rowSlice, artefactTypeId, true);
            std::string fieldTitle = newFld.getTitle();
            // Check if the input data already exists
            auto stmt = db.prepareStatement("SELECT artefactdatafieldid FROM ArtefactDataFields WHERE artefacttypeid = ? AND fieldtitle = ?");
            stmt->setInt(1,artefactTypeId);
            stmt->setString(2,fieldTitle);
            auto res = stmt->executeQuery();
            if (!res->next()) {
                //create a new item
                artefactDataFieldId = addField(db, newFld);
            } else {
                // Update the existing item
                artefactDataFieldId = res->getInt("artefactdatafieldid");
                FieldData existFld(artefactDataFieldId, &db);
                if (row.count("valuetype") && !row.at("valuetype").empty()) existFld.setValue("valuetype", row.at("valuetype"));
                if (row.count("maximumlength") && !row.at("maximumlength").empty()) existFld.setValue("maximumlength", row.at("maximumlength"));
                if (row.count("maximumvalue") && !row.at("maximumvalue").empty()) existFld.setValue("maximumvalue", row.at("maximumvalue"));
                if (row.count("minimumvalue") && !row.at("minimumvalue").empty()) existFld.setValue("minimumvalue", row.at("minimumvalue"));
                setFieldData(db, existFld, artefactDataFieldId);
            } 
        } catch (DataError& e) {
            std::cout<< "Warning: " <<  e.what() << " Skipping item." << std::endl;
            continue;
        }
        // Fields are the terminal node in the artefact type / fields hierarchy
        (void)artefactDataFieldId;
    }
}
void addArtefactDataItemsRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int artefactId, int artefactTypeId) {
    // Check if the required fields are present
    if (!rows[0].count("artefactdatafieldid") || rows[0].at("artefactdatafieldid").empty()) {
        if (!rows[0].count("fieldname") || rows[0].at("fieldname").empty()) return;
    }

    std::vector<std::map<std::string,std::string>> rowSlice;
    int artefactDataId;
    for (const auto& row : rows) {
        try {
            // Try to build a data object from data
            rowSlice = std::vector<std::map<std::string,std::string>>{row};
            ArtefactDataItemData newDat(db, rowSlice, artefactId, artefactTypeId, true);
            int artefactDataFieldId = newDat.getArtefactDataFieldId();
            // Check if the input data already exists
            auto stmt = db.prepareStatement("SELECT artefactdataid FROM ArtefactData WHERE artefactid = ? AND artefactdatafieldid = ?");
            stmt->setInt(1,artefactId);
            stmt->setInt(2,artefactDataFieldId);
            auto res = stmt->executeQuery();
            if (!res->next()) {
                //create a new item
                artefactDataId = addArtefactDataItem(db, newDat);
            } else {
                // Update the existing item
                artefactDataId = res->getInt("artefactdataid");
                ArtefactDataItemData existDat(artefactDataId, &db);
                if (row.count("value") && !row.at("value").empty()) existDat.setValue("value", row.at("value"));
                setArtefactDataItemData(db, existDat, artefactDataId);
            } 
        } catch (DataError& e) {
            std::cout<< "Warning: " <<  e.what() << " Skipping item." << std::endl;
            continue;
        }
        // Artefact data items are the terminal node in the artefact data hierarchy
        (void)artefactDataId;
    }
}

void addMilestonesRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int projectId) {
    // Check if the required fields are present
    if (!rows[0].count("milestonename") || rows[0].at("milestonename").empty()) return;

    std::vector<std::map<std::string,std::string>> rowSlice;
    int milestoneId;
    std::string milestoneName;
    for (const auto& row : rows) {
        try {
            // Try to build a data object from data
            rowSlice = std::vector<std::map<std::string,std::string>>{row};
            MilestoneData newMil(db, rowSlice, projectId, true);
            milestoneName = newMil.getName();
            // Check if the input data already exists
            auto stmt = db.prepareStatement("SELECT milestoneid FROM Milestones WHERE milestonename = ? AND projectid = ?");
            stmt->setString(1,milestoneName);
            stmt->setInt(2,projectId);
            auto res = stmt->executeQuery();
            if (!res->next()) {
                //create a new item
                milestoneId = addMilestone(db, newMil);
            } else {
                // Update the existing item
                milestoneId = res->getInt("milestoneid");
                MilestoneData existMil(milestoneId, &db);
                if (!row.at("milestonename").empty()) existMil.setValue("milestonename", row.at("milestonename"));
                setMilestoneData(db, existMil, milestoneId);
            } 
        } catch (DataError& e) {
            std::cout<< "Warning: " <<  e.what() << " Skipping item." << std::endl;
            continue;
        }
        addMilestoneStepsRecursive(db, rowSlice, milestoneId);
    }
    
}

void addMilestoneStepsRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int milestoneId) {
    // Check if the required fields are present
    if (!rows[0].count("milestonestepname") || rows[0].at("milestonestepname").empty()) return;

    std::vector<std::map<std::string,std::string>> rowSlice;
    int milestoneStepId;
    std::string milestoneStepName;
    for (const auto& row : rows) {
        try {
            // Try to build a data object from data
            rowSlice = std::vector<std::map<std::string,std::string>>{row};
            MilestoneStepData newMilStp(db, rowSlice, milestoneId, true);
            milestoneStepName = newMilStp.getName();
            // Check if the input data already exists
            auto stmt = db.prepareStatement("SELECT milestonestepid FROM MilestoneSteps WHERE milestonestepname = ? AND milestoneid = ?");
            stmt->setString(1,milestoneStepName);
            stmt->setInt(2,milestoneId);
            auto res = stmt->executeQuery();
            if (!res->next()) {
                //create a new item
                milestoneStepId = addMilestoneStep(db, newMilStp);
            } else {
                // Update the existing item if values have changed
                milestoneStepId = res->getInt("milestonestepid");
                MilestoneStepData existMilStp(milestoneStepId, &db);
                if (row.count("progressratio") && !row.at("progressratio").empty()) existMilStp.setValue("progressratio", row.at("progressratio"));
                setMilestoneStepData(db, existMilStp, milestoneStepId);
            } 
        } catch (DataError& e) {
            std::cout<< "Warning: " <<  e.what() << " Skipping item." << std::endl;
            continue;
        }
        // Milestone steps are the terminal node in the milestone hierarchy
        (void)milestoneStepId;
    }
}

} //namespace commands
