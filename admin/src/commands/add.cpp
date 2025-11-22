#include "add.h"

namespace commands {
namespace add {

    void addProject(Database& db, const std::string& projectName, const std::string& projectNumber, int projectManagerId) {
        try {
            // Insert project
            auto insertStmt = db.prepareStatement(
                "INSERT INTO Projects (projectname, projectno, projectmanagerid) VALUES (?, ?, ?)"
            );
            insertStmt->setString(1, projectName);
            insertStmt->setString(2, projectNumber);
            insertStmt->setInt(3, projectManagerId);

            insertStmt->executeUpdate();

            auto lastIdRes = db.executeQuery("SELECT LAST_INSERT_ID() as id");
            if (lastIdRes->next()) {
                std::cout << "Project created successfully with ID: " << lastIdRes->getInt("id") << std::endl;
            }
        } catch (sql::SQLException& e) {
            std::cerr << "Error creating project: " << e.what() << std::endl;
            throw;
        }
    }

    void addCategory(Database& db, int projectId, const std::string& categoryName){
        try{
            // Verify if project exists
            auto checkStmt = db.prepareStatement("SELECT projectid FROM Projects WHERE projectid = ?");
            checkStmt->setInt(1,projectId);
            auto res = checkStmt->executeQuery();

            if(!res->next()) {
                std::cerr << "Project with ID " << projectId << " not found." << std::endl;
                return;
            }

            // Insert new category
            auto insertStmt = db.prepareStatement(
                "INSERT INTO ActivityCategories (projectid, categoryname) VALUES (?, ?)"
            );
            insertStmt->setInt(1, projectId);
            insertStmt->setString(2, categoryName);

            insertStmt->executeUpdate();

            auto lastIdRes = db.executeQuery("SELECT LAST_INSERT_ID() as id");
            if (lastIdRes->next()) {
                std::cout << "Activity category created successfully with ID: " << lastIdRes->getInt("id") << std::endl;
            }
        } catch (sql::SQLException& e) {
            std::cerr << "Error creating activity category: " << e.what() << std::endl;
            throw;
        }
    }

    void addSubcategory(Database& db, int categoryId, const std::string& subCategoryName){
        try{
            // Verify if category exists
            auto checkStmt = db.prepareStatement("SELECT categoryid FROM ActivityCategories WHERE categoryid = ?");
            checkStmt->setInt(1,categoryId);
            auto res = checkStmt->executeQuery();

            if(!res->next()) {
                std::cerr << "Activity category with ID " << categoryId << " not found." << std::endl;
                return;
            }

            // Insert new sub-category
            auto insertStmt = db.prepareStatement(
                "INSERT INTO ActivitySubcategories (categoryid, subcategoryname) VALUES (?, ?)"
            );
            insertStmt->setInt(1, categoryId);
            insertStmt->setString(2, subCategoryName);

            insertStmt->executeUpdate();

            auto lastIdRes = db.executeQuery("SELECT LAST_INSERT_ID() as id");
            if (lastIdRes->next()) {
                std::cout << "Activity sub-category created successfully with ID: " << lastIdRes->getInt("id") << std::endl;
            }
        } catch (sql::SQLException& e) {
            std::cerr << "Error creating activity sub-category: " << e.what() << std::endl;
            throw;
        }
    }
    void addActivity(Database& db, const std::string& name, int subcategoryId,
                    int managerId, const std::string& startDate,
                    const std::string& endDate, double plannedHours) {
        try {
            auto stmt = db.prepareStatement(
                "INSERT INTO Activities (subcategoryid, activityname, activitymanagerid, "
                "plannedstart, plannedfinish, plannedhours) VALUES (?, ?, ?, ?, ?, ?)"
            );
            stmt->setInt(1, subcategoryId);
            stmt->setString(2, name);
            stmt->setInt(3, managerId);
            stmt->setString(4, startDate);
            stmt->setString(5, endDate);
            stmt->setDouble(6, plannedHours);

            stmt->executeUpdate();

            auto res = db.executeQuery("SELECT LAST_INSERT_ID() as id");
            if (res->next()) {
                std::cout << "Activity created successfully with ID: " << res->getInt("id") << std::endl;
            }
        } catch (sql::SQLException& e) {
            std::cerr << "Error creating activity: " << e.what() << std::endl;
            throw;
        }
    }


    void addTask(Database& db, int activityId, const std::string& taskName, int parentTaskId) {
        try {
            // Verify activity exists
            auto checkStmt = db.prepareStatement("SELECT activityid FROM Activities WHERE activityid = ?");
            checkStmt->setInt(1, activityId);
            auto res = checkStmt->executeQuery();

            if (!res->next()) {
                std::cerr << "Activity with ID " << activityId << " not found." << std::endl;
                return;
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
                    std::cerr << "Parent task with ID " << parentTaskId
                              << " not found in activity " << activityId << std::endl;
                    return;
                }
            }

            // Insert task
            auto insertStmt = db.prepareStatement(
                "INSERT INTO ActivityTasks (activityid, taskname, parenttaskid) VALUES (?, ?, ?)"
            );
            insertStmt->setInt(1, activityId);
            insertStmt->setString(2, taskName);

            if (parentTaskId > 0) {
                insertStmt->setInt(3, parentTaskId);
            } else {
                insertStmt->setNull(3, sql::DataType::INTEGER);
            }

            insertStmt->executeUpdate();

            auto lastIdRes = db.executeQuery("SELECT LAST_INSERT_ID() as id");
            if (lastIdRes->next()) {
                std::cout << "Task created successfully with ID: " << lastIdRes->getInt("id") << std::endl;
            }
        } catch (sql::SQLException& e) {
            std::cerr << "Error creating task: " << e.what() << std::endl;
            throw;
        }
    }

} // namespace add
} // namespace commands
