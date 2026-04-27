#include "remove.h"
#include "../exceptions/exceptions.h"
#include <iostream>
#include "../utils/utils.h"

namespace commands {

void remove(Database& db, Path& pth, std::string param, bool suppressConfirmation){

    if (param.empty()) {
        throw NavigationError("Please provide an item name or ID to remove.");
    }

    int id = 0;

    if (utils::isNumber(param)) {
        id = utils::toInt(param);
        if (id == 0) {
            throw NavigationError("Please provide a valid ID to remove.");
        }
    } else {
        // Name-based lookup
        std::string query;
        int parentId = 0;

        switch (pth.current_type()) {
        case Path::PROJECTS_FOLDER:
            query = "SELECT projectid AS id FROM Projects WHERE projectname = ?";
            break;
        case Path::ACTIVITIES_FOLDER:
            query = "SELECT categoryid AS id FROM ActivityCategories WHERE categoryname = ? AND projectid = ?";
            parentId = pth.getIdByType(Path::PROJECT);
            break;
        case Path::CATEGORY:
            query = "SELECT subcategoryid AS id FROM ActivitySubcategories WHERE subcategoryname = ? AND categoryid = ?";
            parentId = pth.current_id();
            break;
        case Path::SUBCATEGORY:
            query = "SELECT activityid AS id FROM Activities WHERE activityname = ? AND subcategoryid = ?";
            parentId = pth.current_id();
            break;
        case Path::TASKS_FOLDER:
            query = "SELECT taskid AS id FROM ActivityTasks WHERE taskname = ? AND activityid = ? AND parenttaskid IS NULL";
            parentId = pth.getIdByType(Path::ACTIVITY);
            break;
        case Path::TASK:
            query = "SELECT taskid AS id FROM ActivityTasks WHERE taskname = ? AND parenttaskid = ?";
            parentId = pth.current_id();
            break;
        case Path::ARTEFACT_TYPES_FOLDER:
            query = "SELECT artefacttypeid AS id FROM ArtefactTypes WHERE artefactname = ? AND projectid = ?";
            parentId = pth.getIdByType(Path::PROJECT);
            break;
        case Path::ARTEFACTS_FOLDER:
            query = "SELECT artefactid AS id FROM Artefacts WHERE artefacttitle = ? AND artefacttypeid = ?";
            parentId = pth.getIdByType(Path::ARTEFACT_TYPE);
            break;
        case Path::FIELDS_FOLDER:
            query = "SELECT artefactdatafieldid AS id FROM ArtefactDataFields WHERE fieldtitle = ? AND artefacttypeid = ?";
            parentId = pth.getIdByType(Path::ARTEFACT_TYPE);
            break;
        case Path::MILESTONES_FOLDER:
            query = "SELECT milestoneid AS id FROM Milestones WHERE milestonename = ? AND projectid = ?";
            parentId = pth.getIdByType(Path::PROJECT);
            break;
        case Path::MILESTONE:
            query = "SELECT milestonestepid AS id FROM MilestoneSteps WHERE milestonestepname = ? AND milestoneid = ?";
            parentId = pth.current_id();
            break;
        case Path::DEPARTMENTS_FOLDER:
            query = "SELECT departmentid AS id FROM Departments WHERE departmentname = ?";
            break;
        case Path::TEAM_FOLDER:
            query = "SELECT ptm.teammemberid AS id FROM ProjectTeamMembers ptm "
                    "JOIN Employees e ON ptm.employeeid = e.employeeid "
                    "WHERE CONCAT(e.firstname, ' ', e.lastname) = ? AND ptm.projectid = ?";
            parentId = pth.getIdByType(Path::PROJECT);
            break;
        case Path::ASSIGNMENTS_FOLDER:
            query = "SELECT ata.assignmentid AS id FROM ActivityTaskAssignments ata "
                    "JOIN Employees e ON ata.userid = e.employeeid "
                    "WHERE CONCAT(e.firstname, ' ', e.lastname) = ? AND ata.taskid = ?";
            parentId = pth.current_id();
            break;
        default:
            throw NavigationError("Cannot delete here.");
        }

        try {
            auto stmt = db.prepareStatement(query);
            stmt->setString(1, param);
            if (parentId != 0) {
                stmt->setInt(2, parentId);
            }
            auto res = stmt->executeQuery();

            if (!res->next()) {
                throw EntityNotFoundError("Item", param);
            }

            id = res->getInt("id");

            if (res->next()) {
                throw DataError("Multiple items found with name \"" + param + "\". Please use the ID instead.");
            }
        } catch (sql::SQLException& e) {
            throw DatabaseError("looking up item by name", e.what());
        }
    }

    // Check if current location is read-only
    switch(pth.current_type()){
    case Path::ROOT:
    case Path::ORGANIZATION_FOLDER:
    case Path::PROJECT:
    case Path::ACTIVITY:
    case Path::TASK:
    case Path::SUBTASK:
    case Path::ARTEFACT_TYPE:
    case Path::ARTEFACT:
        throw NavigationError("Cannot delete here.");
    default:
        break;
    }

    // Ask for confirmation before destructive operation (skip when running from a source file)
    if (!suppressConfirmation) {
        std::string confirmMsg = "Are you sure you want to remove item with ID " + std::to_string(id) + "?";
        if (!utils::confirmAction(confirmMsg)) {
            std::cout << "Operation cancelled." << std::endl;
            return;
        }
    }

    switch(pth.current_type()){
    case Path::PROJECTS_FOLDER:
        removeProject(db, id);
        break;
    case Path::ACTIVITIES_FOLDER:
        removeCategory(db, id);
        break;
    case Path::CATEGORY:
        removeSubCategory(db, id);
        break;
    case Path::SUBCATEGORY:
        removeActivity(db, id);
        break;
    case Path::TASKS_FOLDER:
    case Path::TASK:
        removeTask(db, id);
        break;
    case Path::NOTES_FOLDER:
        removeNote(db, id);
        break;
    case Path::ARTEFACT_TYPES_FOLDER:
        removeArtefactType(db, id);
        break;
    case Path::ARTEFACTS_FOLDER:
        removeArtefact(db, id);
        break;
    case Path::FIELDS_FOLDER:
        removeField(db, id);
        break;
    case Path::MILESTONES_FOLDER:
        removeMilestone(db, id);
        break;
    case Path::MILESTONE:
        removeMilestoneStep(db, id);
        break;
    case Path::DEPARTMENTS_FOLDER:
        removeDepartment(db, id);
        break;
    case Path::DEPARTMENT:
        removeEmployee(db, id);
        break;
    case Path::TEAM_FOLDER:
        removeTeamMember(db, id);
        break;
    case Path::ASSIGNMENTS_FOLDER:
        removeAssignment(db, id);
        break;
    case Path::ASSIGNMENT:
        removeHours(db, id);
        break;
    case Path::DATA_FOLDER:
        removeArtefactDataItem(db, id);
        break;
    case Path::ASSOCIATIONS_FOLDER:
        removeArtefactToActivityLink(db, id);
        break;
    case Path::ARTEFACT_TO_ACTIVITY_LINK:
        removeMilestoneToArtefactLink(db, id);
        break;
    default:
        break;
    }
}

void removeProject(Database& db, int projectId) {
    try {
        // Begin transaction to ensure all deletes succeed or rollback
        db.beginTransaction();

        // Step 1: Delete Hours for tasks in this project's activities
        auto stmt1 = db.prepareStatement(
            "DELETE h FROM Hours h "
            "INNER JOIN ActivityTaskAssignments ata ON h.assignmentid = ata.assignmentid "
            "INNER JOIN ActivityTasks t ON ata.taskid = t.taskid "
            "INNER JOIN Activities a ON t.activityid = a.activityid "
            "INNER JOIN ActivitySubcategories s ON a.subcategoryid = s.subcategoryid "
            "INNER JOIN ActivityCategories c ON s.categoryid = c.categoryid "
            "WHERE c.projectid = ?"
        );
        stmt1->setInt(1, projectId);
        stmt1->executeUpdate();

        // Step 2: Delete ActivityTaskAssignments
        auto stmt2 = db.prepareStatement(
            "DELETE ata FROM ActivityTaskAssignments ata "
            "INNER JOIN ActivityTasks t ON ata.taskid = t.taskid "
            "INNER JOIN Activities a ON t.activityid = a.activityid "
            "INNER JOIN ActivitySubcategories s ON a.subcategoryid = s.subcategoryid "
            "INNER JOIN ActivityCategories c ON s.categoryid = c.categoryid "
            "WHERE c.projectid = ?"
        );
        stmt2->setInt(1, projectId);
        stmt2->executeUpdate();

        // Step 3: Delete ActivityTasks
        auto stmt3 = db.prepareStatement(
            "DELETE t FROM ActivityTasks t "
            "INNER JOIN Activities a ON t.activityid = a.activityid "
            "INNER JOIN ActivitySubcategories s ON a.subcategoryid = s.subcategoryid "
            "INNER JOIN ActivityCategories c ON s.categoryid = c.categoryid "
            "WHERE c.projectid = ?"
        );
        stmt3->setInt(1, projectId);
        stmt3->executeUpdate();

        // Step 4: Delete ActivityNotes
        auto stmt4 = db.prepareStatement(
            "DELETE n FROM ActivityNotes n "
            "INNER JOIN Activities a ON n.activityid = a.activityid "
            "INNER JOIN ActivitySubcategories s ON a.subcategoryid = s.subcategoryid "
            "INNER JOIN ActivityCategories c ON s.categoryid = c.categoryid "
            "WHERE c.projectid = ?"
        );
        stmt4->setInt(1, projectId);
        stmt4->executeUpdate();

        // Step 5: Delete MilestoneToArtefactLink
        auto stmt5 = db.prepareStatement(
            "DELETE mal FROM MilestoneToArtefactLink mal "
            "INNER JOIN ArtefactToActivityLink aal ON mal.artefactlinkid = aal.artefactlinkid "
            "INNER JOIN Activities a ON aal.activityid = a.activityid "
            "INNER JOIN ActivitySubcategories s ON a.subcategoryid = s.subcategoryid "
            "INNER JOIN ActivityCategories c ON s.categoryid = c.categoryid "
            "WHERE c.projectid = ?"
        );
        stmt5->setInt(1, projectId);
        stmt5->executeUpdate();

        // Step 6: Delete ArtefactToActivityLink
        auto stmt6 = db.prepareStatement(
            "DELETE aal FROM ArtefactToActivityLink aal "
            "INNER JOIN Activities a ON aal.activityid = a.activityid "
            "INNER JOIN ActivitySubcategories s ON a.subcategoryid = s.subcategoryid "
            "INNER JOIN ActivityCategories c ON s.categoryid = c.categoryid "
            "WHERE c.projectid = ?"
        );
        stmt6->setInt(1, projectId);
        stmt6->executeUpdate();

        // Step 7: Delete Activities
        auto stmt7 = db.prepareStatement(
            "DELETE a FROM Activities a "
            "INNER JOIN ActivitySubcategories s ON a.subcategoryid = s.subcategoryid "
            "INNER JOIN ActivityCategories c ON s.categoryid = c.categoryid "
            "WHERE c.projectid = ?"
        );
        stmt7->setInt(1, projectId);
        stmt7->executeUpdate();

        // Step 8: Delete ActivitySubcategories
        auto stmt8 = db.prepareStatement(
            "DELETE s FROM ActivitySubcategories s "
            "INNER JOIN ActivityCategories c ON s.categoryid = c.categoryid "
            "WHERE c.projectid = ?"
        );
        stmt8->setInt(1, projectId);
        stmt8->executeUpdate();

        // Step 9: Delete ActivityCategories
        auto stmt9 = db.prepareStatement(
            "DELETE FROM ActivityCategories WHERE projectid = ?"
        );
        stmt9->setInt(1, projectId);
        stmt9->executeUpdate();

        // Step 10: Delete ArtefactData
        auto stmt10 = db.prepareStatement(
            "DELETE ad FROM ArtefactData ad "
            "INNER JOIN Artefacts a ON ad.artefactid = a.artefactid "
            "INNER JOIN ArtefactTypes at ON a.artefacttypeid = at.artefacttypeid "
            "WHERE at.projectid = ?"
        );
        stmt10->setInt(1, projectId);
        stmt10->executeUpdate();

        // Step 11: Delete ArtefactDataFields
        auto stmt11 = db.prepareStatement(
            "DELETE adf FROM ArtefactDataFields adf "
            "INNER JOIN ArtefactTypes at ON adf.artefacttypeid = at.artefacttypeid "
            "WHERE at.projectid = ?"
        );
        stmt11->setInt(1, projectId);
        stmt11->executeUpdate();

        // Step 12: Delete Artefacts
        auto stmt12 = db.prepareStatement(
            "DELETE a FROM Artefacts a "
            "INNER JOIN ArtefactTypes at ON a.artefacttypeid = at.artefacttypeid "
            "WHERE at.projectid = ?"
        );
        stmt12->setInt(1, projectId);
        stmt12->executeUpdate();

        // Step 13: Delete ArtefactTypes
        auto stmt13 = db.prepareStatement(
            "DELETE FROM ArtefactTypes WHERE projectid = ?"
        );
        stmt13->setInt(1, projectId);
        stmt13->executeUpdate();

        // Step 14: Delete MilestoneSteps
        auto stmt14 = db.prepareStatement(
            "DELETE ms FROM MilestoneSteps ms "
            "INNER JOIN Milestones m ON ms.milestoneid = m.milestoneid "
            "WHERE m.projectid = ?"
        );
        stmt14->setInt(1, projectId);
        stmt14->executeUpdate();

        // Step 15: Delete Milestones
        auto stmt15 = db.prepareStatement(
            "DELETE FROM Milestones WHERE projectid = ?"
        );
        stmt15->setInt(1, projectId);
        stmt15->executeUpdate();

        // Step 16: Delete ProjectTeamMembers
        auto stmt16 = db.prepareStatement(
            "DELETE FROM ProjectTeamMembers WHERE projectid = ?"
        );
        stmt16->setInt(1, projectId);
        stmt16->executeUpdate();

        // Step 17: Finally delete the Project
        auto stmt17 = db.prepareStatement(
            "DELETE FROM Projects WHERE projectid = ?"
        );
        stmt17->setInt(1, projectId);
        int rowsAffected = stmt17->executeUpdate();

        // Commit transaction
        db.commit();

        if (rowsAffected > 0) {
            std::cout << "Project " << projectId << " and all related data deleted successfully." << std::endl;
        } else {
            std::cout << "Project " << projectId << " not found." << std::endl;
        }

    } catch (sql::SQLException& e) {
        // Rollback transaction on error
        db.rollback();
        throw DatabaseError("deleting project", e.what());
    }
}

void removeCategory(Database& db, int categoryId) {
    try {
        // Begin transaction to ensure all deletes succeed or rollback
        db.beginTransaction();

        // Step 1: Delete Hours for tasks in this category's activities
        auto stmt1 = db.prepareStatement(
            "DELETE h FROM Hours h "
            "INNER JOIN ActivityTaskAssignments ata ON h.assignmentid = ata.assignmentid "
            "INNER JOIN ActivityTasks t ON ata.taskid = t.taskid "
            "INNER JOIN Activities a ON t.activityid = a.activityid "
            "INNER JOIN ActivitySubcategories s ON a.subcategoryid = s.subcategoryid "
            "WHERE s.categoryid = ?"
        );
        stmt1->setInt(1, categoryId);
        stmt1->executeUpdate();

        // Step 2: Delete ActivityTaskAssignments
        auto stmt2 = db.prepareStatement(
            "DELETE ata FROM ActivityTaskAssignments ata "
            "INNER JOIN ActivityTasks t ON ata.taskid = t.taskid "
            "INNER JOIN Activities a ON t.activityid = a.activityid "
            "INNER JOIN ActivitySubcategories s ON a.subcategoryid = s.subcategoryid "
            "WHERE s.categoryid = ?"
        );
        stmt2->setInt(1, categoryId);
        stmt2->executeUpdate();

        // Step 3: Delete ActivityTasks
        auto stmt3 = db.prepareStatement(
            "DELETE t FROM ActivityTasks t "
            "INNER JOIN Activities a ON t.activityid = a.activityid "
            "INNER JOIN ActivitySubcategories s ON a.subcategoryid = s.subcategoryid "
            "WHERE s.categoryid = ?"
        );
        stmt3->setInt(1, categoryId);
        stmt3->executeUpdate();

        // Step 4: Delete ActivityNotes
        auto stmt4 = db.prepareStatement(
            "DELETE n FROM ActivityNotes n "
            "INNER JOIN Activities a ON n.activityid = a.activityid "
            "INNER JOIN ActivitySubcategories s ON a.subcategoryid = s.subcategoryid "
            "WHERE s.categoryid = ?"
        );
        stmt4->setInt(1, categoryId);
        stmt4->executeUpdate();

        // Step 5: Delete MilestoneToArtefactLink
        auto stmt5 = db.prepareStatement(
            "DELETE mal FROM MilestoneToArtefactLink mal "
            "INNER JOIN ArtefactToActivityLink aal ON mal.artefactlinkid = aal.artefactlinkid "
            "INNER JOIN Activities a ON aal.activityid = a.activityid "
            "INNER JOIN ActivitySubcategories s ON a.subcategoryid = s.subcategoryid "
            "WHERE s.categoryid = ?"
        );
        stmt5->setInt(1, categoryId);
        stmt5->executeUpdate();

        // Step 6: Delete ArtefactToActivityLink
        auto stmt6 = db.prepareStatement(
            "DELETE aal FROM ArtefactToActivityLink aal "
            "INNER JOIN Activities a ON aal.activityid = a.activityid "
            "INNER JOIN ActivitySubcategories s ON a.subcategoryid = s.subcategoryid "
            "WHERE s.categoryid = ?"
        );
        stmt6->setInt(1, categoryId);
        stmt6->executeUpdate();

        // Step 7: Delete Activities
        auto stmt7 = db.prepareStatement(
            "DELETE a FROM Activities a "
            "INNER JOIN ActivitySubcategories s ON a.subcategoryid = s.subcategoryid "
            "WHERE s.categoryid = ?"
        );
        stmt7->setInt(1, categoryId);
        stmt7->executeUpdate();

        // Step 8: Delete ActivitySubcategories
        auto stmt8 = db.prepareStatement(
            "DELETE FROM ActivitySubcategories WHERE categoryid = ?"
        );
        stmt8->setInt(1, categoryId);
        stmt8->executeUpdate();

        // Step 9: Finally delete the Category
        auto stmt9 = db.prepareStatement(
            "DELETE FROM ActivityCategories WHERE categoryid = ?"
        );
        stmt9->setInt(1, categoryId);
        int rowsAffected = stmt9->executeUpdate();

        // Commit transaction
        db.commit();

        if (rowsAffected > 0) {
            std::cout << "Category " << categoryId << " and all related data deleted successfully." << std::endl;
        } else {
            std::cout << "Category " << categoryId << " not found." << std::endl;
        }

    } catch (sql::SQLException& e) {
        // Rollback transaction on error
        db.rollback();
        throw DatabaseError("deleting category", e.what());
    }
}

void removeSubCategory(Database& db, int subCategoryId){
    try {
        // Begin transaction to ensure all deletes succeed or rollback
        db.beginTransaction();

        // Step 1: Delete Hours for tasks in this subcategory's activities
        auto stmt1 = db.prepareStatement(
            "DELETE h FROM Hours h "
            "INNER JOIN ActivityTaskAssignments ata ON h.assignmentid = ata.assignmentid "
            "INNER JOIN ActivityTasks t ON ata.taskid = t.taskid "
            "INNER JOIN Activities a ON t.activityid = a.activityid "
            "WHERE a.subcategoryid = ?"
        );
        stmt1->setInt(1, subCategoryId);
        stmt1->executeUpdate();

        // Step 2: Delete ActivityTaskAssignments
        auto stmt2 = db.prepareStatement(
            "DELETE ata FROM ActivityTaskAssignments ata "
            "INNER JOIN ActivityTasks t ON ata.taskid = t.taskid "
            "INNER JOIN Activities a ON t.activityid = a.activityid "
            "WHERE a.subcategoryid = ?"
        );
        stmt2->setInt(1, subCategoryId);
        stmt2->executeUpdate();

        // Step 3: Delete ActivityTasks
        auto stmt3 = db.prepareStatement(
            "DELETE t FROM ActivityTasks t "
            "INNER JOIN Activities a ON t.activityid = a.activityid "
            "WHERE a.subcategoryid = ?"
        );
        stmt3->setInt(1, subCategoryId);
        stmt3->executeUpdate();

        // Step 4: Delete ActivityNotes
        auto stmt4 = db.prepareStatement(
            "DELETE n FROM ActivityNotes n "
            "INNER JOIN Activities a ON n.activityid = a.activityid "
            "WHERE a.subcategoryid = ?"
        );
        stmt4->setInt(1, subCategoryId);
        stmt4->executeUpdate();

        // Step 5: Delete MilestoneToArtefactLink
        auto stmt5 = db.prepareStatement(
            "DELETE mal FROM MilestoneToArtefactLink mal "
            "INNER JOIN ArtefactToActivityLink aal ON mal.artefactlinkid = aal.artefactlinkid "
            "INNER JOIN Activities a ON aal.activityid = a.activityid "
            "WHERE a.subcategoryid = ?"
        );
        stmt5->setInt(1, subCategoryId);
        stmt5->executeUpdate();

        // Step 6: Delete ArtefactToActivityLink
        auto stmt6 = db.prepareStatement(
            "DELETE aal FROM ArtefactToActivityLink aal "
            "INNER JOIN Activities a ON aal.activityid = a.activityid "
            "WHERE a.subcategoryid = ?"
        );
        stmt6->setInt(1, subCategoryId);
        stmt6->executeUpdate();

        // Step 7: Delete Activities
        auto stmt7 = db.prepareStatement(
            "DELETE FROM Activities WHERE subcategoryid = ?"
        );
        stmt7->setInt(1, subCategoryId);
        stmt7->executeUpdate();

        // Step 8: Finally delete the Subcategory
        auto stmt8 = db.prepareStatement(
            "DELETE FROM ActivitySubcategories WHERE subcategoryid = ?"
        );
        stmt8->setInt(1, subCategoryId);
        int rowsAffected = stmt8->executeUpdate();

        // Commit transaction
        db.commit();

        if (rowsAffected > 0) {
            std::cout << "Subcategory " << subCategoryId << " and all related data deleted successfully." << std::endl;
        } else {
            std::cout << "Subcategory " << subCategoryId << " not found." << std::endl;
        }

    } catch (sql::SQLException& e) {
        // Rollback transaction on error
        db.rollback();
        throw DatabaseError("deleting subcategory", e.what());
    }
}

void removeActivity(Database& db, int activityId) {
    try {
        // Begin transaction to ensure all deletes succeed or rollback
        db.beginTransaction();

        // Step 1: Delete Hours for tasks in this activity
        auto stmt1 = db.prepareStatement(
            "DELETE h FROM Hours h "
            "INNER JOIN ActivityTaskAssignments ata ON h.assignmentid = ata.assignmentid "
            "INNER JOIN ActivityTasks t ON ata.taskid = t.taskid "
            "WHERE t.activityid = ?"
        );
        stmt1->setInt(1, activityId);
        stmt1->executeUpdate();

        // Step 2: Delete ActivityTaskAssignments
        auto stmt2 = db.prepareStatement(
            "DELETE ata FROM ActivityTaskAssignments ata "
            "INNER JOIN ActivityTasks t ON ata.taskid = t.taskid "
            "WHERE t.activityid = ?"
        );
        stmt2->setInt(1, activityId);
        stmt2->executeUpdate();

        // Step 3: Delete ActivityTasks
        auto stmt3 = db.prepareStatement(
            "DELETE FROM ActivityTasks WHERE activityid = ?"
        );
        stmt3->setInt(1, activityId);
        stmt3->executeUpdate();

        // Step 4: Delete ActivityNotes
        auto stmt4 = db.prepareStatement(
            "DELETE FROM ActivityNotes WHERE activityid = ?"
        );
        stmt4->setInt(1, activityId);
        stmt4->executeUpdate();

        // Step 5: Delete MilestoneToArtefactLink
        auto stmt5 = db.prepareStatement(
            "DELETE mal FROM MilestoneToArtefactLink mal "
            "INNER JOIN ArtefactToActivityLink aal ON mal.artefactlinkid = aal.artefactlinkid "
            "WHERE aal.activityid = ?"
        );
        stmt5->setInt(1, activityId);
        stmt5->executeUpdate();

        // Step 6: Delete ArtefactToActivityLink
        auto stmt6 = db.prepareStatement(
            "DELETE FROM ArtefactToActivityLink WHERE activityid = ?"
        );
        stmt6->setInt(1, activityId);
        stmt6->executeUpdate();

        // Step 7: Finally delete the Activity
        auto stmt7 = db.prepareStatement(
            "DELETE FROM Activities WHERE activityid = ?"
        );
        stmt7->setInt(1, activityId);
        int rowsAffected = stmt7->executeUpdate();

        // Commit transaction
        db.commit();

        if (rowsAffected > 0) {
            std::cout << "Activity " << activityId << " and all related data deleted successfully." << std::endl;
        } else {
            std::cout << "Activity " << activityId << " not found." << std::endl;
        }

    } catch (sql::SQLException& e) {
        // Rollback transaction on error
        db.rollback();
        throw DatabaseError("deleting activity", e.what());
    }
}

void removeTask(Database& db, int taskId) {
    try {
        // Begin transaction to ensure all deletes succeed or rollback
        db.beginTransaction();

        // Step 1: Delete all child tasks recursively (tasks where parenttaskid = taskId)
        // First, we need to handle the hierarchy - delete children before parent
        auto stmt1 = db.prepareStatement(
            "DELETE h FROM Hours h "
            "INNER JOIN ActivityTaskAssignments ata ON h.assignmentid = ata.assignmentid "
            "WHERE ata.taskid IN (SELECT taskid FROM ActivityTasks WHERE parenttaskid = ?)"
        );
        stmt1->setInt(1, taskId);
        stmt1->executeUpdate();

        // Step 2: Delete ActivityTaskAssignments for child tasks
        auto stmt2 = db.prepareStatement(
            "DELETE FROM ActivityTaskAssignments WHERE taskid IN "
            "(SELECT taskid FROM ActivityTasks WHERE parenttaskid = ?)"
        );
        stmt2->setInt(1, taskId);
        stmt2->executeUpdate();

        // Step 3: Delete child tasks
        auto stmt3 = db.prepareStatement(
            "DELETE FROM ActivityTasks WHERE parenttaskid = ?"
        );
        stmt3->setInt(1, taskId);
        stmt3->executeUpdate();

        // Step 4: Delete Hours for this task
        auto stmt4 = db.prepareStatement(
            "DELETE h FROM Hours h "
            "INNER JOIN ActivityTaskAssignments ata ON h.assignmentid = ata.assignmentid "
            "WHERE ata.taskid = ?"
        );
        stmt4->setInt(1, taskId);
        stmt4->executeUpdate();

        // Step 5: Delete ActivityTaskAssignments for this task
        auto stmt5 = db.prepareStatement(
            "DELETE FROM ActivityTaskAssignments WHERE taskid = ?"
        );
        stmt5->setInt(1, taskId);
        stmt5->executeUpdate();

        // Step 6: Finally delete the Task
        auto stmt6 = db.prepareStatement(
            "DELETE FROM ActivityTasks WHERE taskid = ?"
        );
        stmt6->setInt(1, taskId);
        int rowsAffected = stmt6->executeUpdate();

        // Commit transaction
        db.commit();

        if (rowsAffected > 0) {
            std::cout << "Task " << taskId << " and all related data deleted successfully." << std::endl;
        } else {
            std::cout << "Task " << taskId << " not found." << std::endl;
        }

    } catch (sql::SQLException& e) {
        // Rollback transaction on error
        db.rollback();
        throw DatabaseError("deleting task", e.what());
    }
}

void removeEmployee(Database& db, int id) {
    try {
        db.beginTransaction();

        // Check if employee exists
        auto checkStmt = db.prepareStatement("SELECT firstname, lastname FROM Employees WHERE employeeid = ?");
        checkStmt->setInt(1, id);
        auto res = checkStmt->executeQuery();

        if (!res->next()) {
            db.rollback();
            throw EntityNotFoundError("Employee", id);
        }

        std::string name = res->getString("firstname") + " " + res->getString("lastname");

        // Check if employee is an activity manager
        auto amCheckStmt = db.prepareStatement(
            "SELECT COUNT(*) as count FROM Activities WHERE activitymanagerid = ?"
        );
        amCheckStmt->setInt(1, id);
        auto amRes = amCheckStmt->executeQuery();
        if (amRes->next() && amRes->getInt("count") > 0) {
            int count = amRes->getInt("count");
            db.rollback();
            throw ConstraintViolationError("Cannot delete employee " + name + " - assigned as manager for " + std::to_string(count) + " activities.");
        }

        // Delete employee
        auto deleteStmt = db.prepareStatement("DELETE FROM Employees WHERE employeeid = ?");
        deleteStmt->setInt(1, id);
        deleteStmt->executeUpdate();

        db.commit();
        std::cout << "Employee " << name << " deleted successfully." << std::endl;

    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("deleting employee", e.what());
    }
}
void removeNote(Database& db, int noteId){
    try {
        // Begin transaction
        db.beginTransaction();

        // Delete the note
        auto stmt = db.prepareStatement(
            "DELETE FROM ActivityNotes WHERE noteid = ?"
        );
        stmt->setInt(1, noteId);
        int rowsAffected = stmt->executeUpdate();

        // Commit transaction
        db.commit();

        if (rowsAffected > 0) {
            std::cout << "Note " << noteId << " deleted successfully." << std::endl;
        } else {
            std::cout << "Note " << noteId << " not found." << std::endl;
        }

    } catch (sql::SQLException& e) {
        // Rollback transaction on error
        db.rollback();
        throw DatabaseError("deleting note", e.what());
    }
}
void removeArtefactType(Database& db, int artefactTypeId){
    try {
        // Begin transaction to ensure all deletes succeed or rollback
        db.beginTransaction();

        // Step 1: Delete MilestoneToArtefactLink
        auto stmt1 = db.prepareStatement(
            "DELETE mal FROM MilestoneToArtefactLink mal "
            "INNER JOIN ArtefactToActivityLink aal ON mal.artefactlinkid = aal.artefactlinkid "
            "INNER JOIN Artefacts a ON aal.artefactid = a.artefactid "
            "WHERE a.artefacttypeid = ?"
        );
        stmt1->setInt(1, artefactTypeId);
        stmt1->executeUpdate();

        // Step 2: Delete ArtefactToActivityLink
        auto stmt2 = db.prepareStatement(
            "DELETE aal FROM ArtefactToActivityLink aal "
            "INNER JOIN Artefacts a ON aal.artefactid = a.artefactid "
            "WHERE a.artefacttypeid = ?"
        );
        stmt2->setInt(1, artefactTypeId);
        stmt2->executeUpdate();

        // Step 3: Delete ArtefactData
        auto stmt3 = db.prepareStatement(
            "DELETE ad FROM ArtefactData ad "
            "INNER JOIN Artefacts a ON ad.artefactid = a.artefactid "
            "WHERE a.artefacttypeid = ?"
        );
        stmt3->setInt(1, artefactTypeId);
        stmt3->executeUpdate();

        // Step 4: Delete Artefacts
        auto stmt4 = db.prepareStatement(
            "DELETE FROM Artefacts WHERE artefacttypeid = ?"
        );
        stmt4->setInt(1, artefactTypeId);
        stmt4->executeUpdate();

        // Step 5: Delete ArtefactDataFields
        auto stmt5 = db.prepareStatement(
            "DELETE FROM ArtefactDataFields WHERE artefacttypeid = ?"
        );
        stmt5->setInt(1, artefactTypeId);
        stmt5->executeUpdate();

        // Step 6: Finally delete the ArtefactType
        auto stmt6 = db.prepareStatement(
            "DELETE FROM ArtefactTypes WHERE artefacttypeid = ?"
        );
        stmt6->setInt(1, artefactTypeId);
        int rowsAffected = stmt6->executeUpdate();

        // Commit transaction
        db.commit();

        if (rowsAffected > 0) {
            std::cout << "Artefact type " << artefactTypeId << " and all related data deleted successfully." << std::endl;
        } else {
            std::cout << "Artefact type " << artefactTypeId << " not found." << std::endl;
        }

    } catch (sql::SQLException& e) {
        // Rollback transaction on error
        db.rollback();
        throw DatabaseError("deleting artefact type", e.what());
    }
}
void removeArtefact(Database& db, int artefactId){
    try {
        // Begin transaction to ensure all deletes succeed or rollback
        db.beginTransaction();

        // Step 1: Delete MilestoneToArtefactLink
        auto stmt1 = db.prepareStatement(
            "DELETE mal FROM MilestoneToArtefactLink mal "
            "INNER JOIN ArtefactToActivityLink aal ON mal.artefactlinkid = aal.artefactlinkid "
            "WHERE aal.artefactid = ?"
        );
        stmt1->setInt(1, artefactId);
        stmt1->executeUpdate();

        // Step 2: Delete ArtefactToActivityLink
        auto stmt2 = db.prepareStatement(
            "DELETE FROM ArtefactToActivityLink WHERE artefactid = ?"
        );
        stmt2->setInt(1, artefactId);
        stmt2->executeUpdate();

        // Step 3: Delete ArtefactData
        auto stmt3 = db.prepareStatement(
            "DELETE FROM ArtefactData WHERE artefactid = ?"
        );
        stmt3->setInt(1, artefactId);
        stmt3->executeUpdate();

        // Step 4: Finally delete the Artefact
        auto stmt4 = db.prepareStatement(
            "DELETE FROM Artefacts WHERE artefactid = ?"
        );
        stmt4->setInt(1, artefactId);
        int rowsAffected = stmt4->executeUpdate();

        // Commit transaction
        db.commit();

        if (rowsAffected > 0) {
            std::cout << "Artefact " << artefactId << " and all related data deleted successfully." << std::endl;
        } else {
            std::cout << "Artefact " << artefactId << " not found." << std::endl;
        }

    } catch (sql::SQLException& e) {
        // Rollback transaction on error
        db.rollback();
        throw DatabaseError("deleting artefact", e.what());
    }
}
void removeField(Database& db, int fieldId){
    try {
        // Begin transaction to ensure all deletes succeed or rollback
        db.beginTransaction();

        // Step 1: Delete ArtefactData for this field
        auto stmt1 = db.prepareStatement(
            "DELETE FROM ArtefactData WHERE artefactdatafieldid = ?"
        );
        stmt1->setInt(1, fieldId);
        stmt1->executeUpdate();

        // Step 2: Finally delete the Field
        auto stmt2 = db.prepareStatement(
            "DELETE FROM ArtefactDataFields WHERE artefactdatafieldid = ?"
        );
        stmt2->setInt(1, fieldId);
        int rowsAffected = stmt2->executeUpdate();

        // Commit transaction
        db.commit();

        if (rowsAffected > 0) {
            std::cout << "Field " << fieldId << " and all related data deleted successfully." << std::endl;
        } else {
            std::cout << "Field " << fieldId << " not found." << std::endl;
        }

    } catch (sql::SQLException& e) {
        // Rollback transaction on error
        db.rollback();
        throw DatabaseError("deleting field", e.what());
    }
}
void removeMilestone(Database& db, int milestoneId){
    try {
        // Begin transaction to ensure all deletes succeed or rollback
        db.beginTransaction();

        // Step 1: Delete MilestoneToArtefactLink
        auto stmt1 = db.prepareStatement(
            "DELETE mal FROM MilestoneToArtefactLink mal "
            "INNER JOIN MilestoneSteps ms ON mal.milestonestepid = ms.milestonestepid "
            "WHERE ms.milestoneid = ?"
        );
        stmt1->setInt(1, milestoneId);
        stmt1->executeUpdate();

        // Step 2: Delete MilestoneSteps
        auto stmt2 = db.prepareStatement(
            "DELETE FROM MilestoneSteps WHERE milestoneid = ?"
        );
        stmt2->setInt(1, milestoneId);
        stmt2->executeUpdate();

        // Step 3: Finally delete the Milestone
        auto stmt3 = db.prepareStatement(
            "DELETE FROM Milestones WHERE milestoneid = ?"
        );
        stmt3->setInt(1, milestoneId);
        int rowsAffected = stmt3->executeUpdate();

        // Commit transaction
        db.commit();

        if (rowsAffected > 0) {
            std::cout << "Milestone " << milestoneId << " and all related data deleted successfully." << std::endl;
        } else {
            std::cout << "Milestone " << milestoneId << " not found." << std::endl;
        }

    } catch (sql::SQLException& e) {
        // Rollback transaction on error
        db.rollback();
        throw DatabaseError("deleting milestone", e.what());
    }
}
void removeMilestoneStep(Database& db, int milestoneStepId){
    try {
        // Begin transaction to ensure all deletes succeed or rollback
        db.beginTransaction();

        // Step 1: Delete MilestoneToArtefactLink
        auto stmt1 = db.prepareStatement(
            "DELETE FROM MilestoneToArtefactLink WHERE milestonestepid = ?"
        );
        stmt1->setInt(1, milestoneStepId);
        stmt1->executeUpdate();

        // Step 2: Finally delete the MilestoneStep
        auto stmt2 = db.prepareStatement(
            "DELETE FROM MilestoneSteps WHERE milestonestepid = ?"
        );
        stmt2->setInt(1, milestoneStepId);
        int rowsAffected = stmt2->executeUpdate();

        // Commit transaction
        db.commit();

        if (rowsAffected > 0) {
            std::cout << "Milestone step " << milestoneStepId << " and all related data deleted successfully." << std::endl;
        } else {
            std::cout << "Milestone step " << milestoneStepId << " not found." << std::endl;
        }

    } catch (sql::SQLException& e) {
        // Rollback transaction on error
        db.rollback();
        throw DatabaseError("deleting milestone step", e.what());
    }
}

void removeDepartment(Database& db, int departmentId) {
    try {
        db.beginTransaction();

        // Check if any employees are in this department
        auto empCheck = db.prepareStatement("SELECT COUNT(*) as count FROM Employees WHERE departmentid = ?");
        empCheck->setInt(1, departmentId);
        auto empRes = empCheck->executeQuery();
        if (empRes->next() && empRes->getInt("count") > 0) {
            db.rollback();
            throw ConstraintViolationError("Cannot delete department. It has employees assigned.");
        }

        auto stmt = db.prepareStatement("DELETE FROM Departments WHERE departmentid = ?");
        stmt->setInt(1, departmentId);
        int rowsAffected = stmt->executeUpdate();

        db.commit();

        if (rowsAffected > 0) {
            std::cout << "Department " << departmentId << " deleted successfully." << std::endl;
        } else {
            std::cout << "Department " << departmentId << " not found." << std::endl;
        }
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("deleting department", e.what());
    }
}

void removeTeamMember(Database& db, int teamMemberId) {
    try {
        db.beginTransaction();

        auto stmt = db.prepareStatement("DELETE FROM ProjectTeamMembers WHERE teammemberid = ?");
        stmt->setInt(1, teamMemberId);
        int rowsAffected = stmt->executeUpdate();

        db.commit();

        if (rowsAffected > 0) {
            std::cout << "Team member " << teamMemberId << " removed successfully." << std::endl;
        } else {
            std::cout << "Team member " << teamMemberId << " not found." << std::endl;
        }
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("removing team member", e.what());
    }
}

void removeAssignment(Database& db, int assignmentId) {
    try {
        db.beginTransaction();

        // Step 1: Delete hours for this assignment
        auto stmt1 = db.prepareStatement("DELETE FROM Hours WHERE assignmentid = ?");
        stmt1->setInt(1, assignmentId);
        stmt1->executeUpdate();

        // Step 2: Delete the assignment
        auto stmt2 = db.prepareStatement("DELETE FROM ActivityTaskAssignments WHERE assignmentid = ?");
        stmt2->setInt(1, assignmentId);
        int rowsAffected = stmt2->executeUpdate();

        db.commit();

        if (rowsAffected > 0) {
            std::cout << "Assignment " << assignmentId << " and all related hours deleted successfully." << std::endl;
        } else {
            std::cout << "Assignment " << assignmentId << " not found." << std::endl;
        }
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("deleting assignment", e.what());
    }
}

void removeHours(Database& db, int bookingId) {
    try {
        db.beginTransaction();

        auto stmt = db.prepareStatement("DELETE FROM Hours WHERE bookingid = ?");
        stmt->setInt(1, bookingId);
        int rowsAffected = stmt->executeUpdate();

        db.commit();

        if (rowsAffected > 0) {
            std::cout << "Hours booking " << bookingId << " deleted successfully." << std::endl;
        } else {
            std::cout << "Hours booking " << bookingId << " not found." << std::endl;
        }
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("deleting hours", e.what());
    }
}

void removeArtefactDataItem(Database& db, int artefactDataId) {
    try {
        db.beginTransaction();

        auto stmt = db.prepareStatement("DELETE FROM ArtefactData WHERE artefactdataid = ?");
        stmt->setInt(1, artefactDataId);
        int rowsAffected = stmt->executeUpdate();

        db.commit();

        if (rowsAffected > 0) {
            std::cout << "Artefact data item " << artefactDataId << " deleted successfully." << std::endl;
        } else {
            std::cout << "Artefact data item " << artefactDataId << " not found." << std::endl;
        }
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("deleting artefact data", e.what());
    }
}

void removeArtefactToActivityLink(Database& db, int artefactLinkId) {
    try {
        db.beginTransaction();

        // Step 1: Delete MilestoneToArtefactLink
        auto stmt1 = db.prepareStatement("DELETE FROM MilestoneToArtefactLink WHERE artefactlinkid = ?");
        stmt1->setInt(1, artefactLinkId);
        stmt1->executeUpdate();

        // Step 2: Delete the link
        auto stmt2 = db.prepareStatement("DELETE FROM ArtefactToActivityLink WHERE artefactlinkid = ?");
        stmt2->setInt(1, artefactLinkId);
        int rowsAffected = stmt2->executeUpdate();

        db.commit();

        if (rowsAffected > 0) {
            std::cout << "Artefact-activity link " << artefactLinkId << " and related data deleted successfully." << std::endl;
        } else {
            std::cout << "Artefact-activity link " << artefactLinkId << " not found." << std::endl;
        }
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("deleting artefact-activity link", e.what());
    }
}

void removeMilestoneToArtefactLink(Database& db, int milestoneLinkId) {
    try {
        db.beginTransaction();

        auto stmt = db.prepareStatement("DELETE FROM MilestoneToArtefactLink WHERE milestonelinkid = ?");
        stmt->setInt(1, milestoneLinkId);
        int rowsAffected = stmt->executeUpdate();

        db.commit();

        if (rowsAffected > 0) {
            std::cout << "Milestone-artefact link " << milestoneLinkId << " deleted successfully." << std::endl;
        } else {
            std::cout << "Milestone-artefact link " << milestoneLinkId << " not found." << std::endl;
        }
    } catch (sql::SQLException& e) {
        db.rollback();
        throw DatabaseError("deleting milestone-artefact link", e.what());
    }
}

} // namespace commands
