#include "remove.h"
#include <iostream>

namespace commands {
namespace remove {

    void removeProject(Database& db, int projectId) {
        try {
            // Begin transaction to ensure all deletes succeed or rollback
            db.beginTransaction();

            // Step 1: Delete Hours for tasks in this project's activities
            auto stmt1 = db.prepareStatement(
                "DELETE h FROM Hours h "
                "INNER JOIN ActivityTasks t ON h.taskid = t.taskid "
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

            // Step 14: Delete MilestonesSteps
            auto stmt14 = db.prepareStatement(
                "DELETE ms FROM MilestonesSteps ms "
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

            // Step 16: Finally delete the Project
            auto stmt16 = db.prepareStatement(
                "DELETE FROM Projects WHERE projectid = ?"
            );
            stmt16->setInt(1, projectId);
            int rowsAffected = stmt16->executeUpdate();

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
            std::cerr << "Error deleting project: " << e.what() << std::endl;
            throw;
        }
    }

    void removeCategory(Database& db, int categoryId) {
        try {
            // Begin transaction to ensure all deletes succeed or rollback
            db.beginTransaction();

            // Step 1: Delete Hours for tasks in this category's activities
            auto stmt1 = db.prepareStatement(
                "DELETE h FROM Hours h "
                "INNER JOIN ActivityTasks t ON h.taskid = t.taskid "
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
            std::cerr << "Error deleting category: " << e.what() << std::endl;
            throw;
        }
    }

    void removeSubCategory(Database& db, int subCategoryId){
        try {
            // Begin transaction to ensure all deletes succeed or rollback
            db.beginTransaction();

            // Step 1: Delete Hours for tasks in this subcategory's activities
            auto stmt1 = db.prepareStatement(
                "DELETE h FROM Hours h "
                "INNER JOIN ActivityTasks t ON h.taskid = t.taskid "
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
            std::cerr << "Error deleting subcategory: " << e.what() << std::endl;
            throw;
        }
    }

    void removeActivity(Database& db, int activityId) {
        try {
            // Begin transaction to ensure all deletes succeed or rollback
            db.beginTransaction();

            // Step 1: Delete Hours for tasks in this activity
            auto stmt1 = db.prepareStatement(
                "DELETE h FROM Hours h "
                "INNER JOIN ActivityTasks t ON h.taskid = t.taskid "
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
            std::cerr << "Error deleting activity: " << e.what() << std::endl;
            throw;
        }
    }

    void removeTask(Database& db, int taskId) {
        try {
            // Begin transaction to ensure all deletes succeed or rollback
            db.beginTransaction();

            // Step 1: Delete all child tasks recursively (tasks where parenttaskid = taskId)
            // First, we need to handle the hierarchy - delete children before parent
            auto stmt1 = db.prepareStatement(
                "DELETE FROM Hours WHERE taskid IN "
                "(SELECT taskid FROM ActivityTasks WHERE parenttaskid = ?)"
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
                "DELETE FROM Hours WHERE taskid = ?"
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
            std::cerr << "Error deleting task: " << e.what() << std::endl;
            throw;
        }
    }

} // namespace remove
} // namespace commands
