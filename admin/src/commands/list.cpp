#include "list.h"

#include <iostream>
#include <iomanip>
#include <string>

namespace commands {
namespace list {


    void listActivities(Database& db, int projectId) {
        std::string query = R"(
            SELECT a.activityid, a.activityname, a.plannedstart, a.plannedfinish,
                   CONCAT(e.firstname, ' ', e.lastname) as manager,
                   ac.categoryname, acs.subcategoryname
            FROM Activities a
            JOIN Employees e ON a.activitymanagerid = e.employeeid
            JOIN ActivitySubcategories acs ON a.subcategoryid = acs.subcategoryid
            JOIN ActivityCategories ac ON acs.categoryid = ac.categoryid
        )";

        std::unique_ptr<sql::ResultSet> res;
        if (projectId > 0) {
            query += " WHERE ac.projectid = ? ORDER BY a.activityid";
            auto stmt = db.prepareStatement(query);
            stmt->setInt(1, projectId);
            res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
        } else {
            query += " ORDER BY a.activityid";
            res = db.executeQuery(query);
        }

        std::cout << "\n" << std::string(120, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(40) << "Activity Name"
                  << std::setw(15) << "Start Date"
                  << std::setw(15) << "End Date"
                  << std::setw(20) << "Manager"
                  << std::endl;
        std::cout << std::string(120, '-') << std::endl;

        int count = 0;
        while (res->next()) {
            std::cout << std::left << std::setw(5) << res->getInt("activityid")
                      << std::setw(40) << res->getString("activityname").substr(0, 39)
                      << std::setw(15) << res->getString("plannedstart")
                      << std::setw(15) << res->getString("plannedfinish")
                      << std::setw(20) << res->getString("manager")
                      << std::endl;
            count++;
        }

        std::cout << std::string(120, '=') << std::endl;
        std::cout << "Total: " << count << " activities" << std::endl;
    }

    void listCategories(Database& db, int projectId){
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
        std::cout << "\n" << std::string(70,'=') <<'\n';
        std::cout << std::left << std::setw(5) << "ID"
                    <<std::setw(40) << "Activity Category" << std::endl;
        std::cout << std::string(70,'-') << std::endl;

        int count = 0;
        while (res->next()) {
            std::cout << std::left << std::setw(5) << res->getInt("categoryid")
                << std::setw(40) << res->getString("categoryname").substr(0,39)
                << std::endl;
            count ++;
        }
        std::cout << std::string(70, '=') << std::endl;
        std::cout << "Total: " << count << " activity categories " << std::endl;
    }

    void listSubcategories(Database& db, int categoryId){
        std::string query = R"(
            SELECT s.subcategoryid, s.subcategoryname
            FROM ActivitySubcategories s
            WHERE categoryid = ?
        )";
        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, categoryId);
        auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
        std::cout << "\n" << std::string(70,'=') <<'\n';
        std::cout << std::left << std::setw(5) << "ID"
                    <<std::setw(40) << "Activity Sub-category" << std::endl;
        std::cout << std::string(70,'-') << std::endl;

        int count = 0;
        while (res->next()) {
            std::cout << std::left << std::setw(5) << res->getInt("subcategoryid")
                << std::setw(40) << res->getString("subcategoryname").substr(0,39)
                << std::endl;
            count ++;
        }
        std::cout << std::string(70, '=') << std::endl;
        std::cout << "Total: " << count << " activity sub-categories " << std::endl;
    }

    void listProjects(Database& db){
        std::string query = R"(
            SELECT p.projectid, p.projectname
            FROM Projects p
        )";
        auto res = db.executeQuery(query);
        std::cout << "\n" << std::string(70,'=') <<'\n';
        std::cout << std::left << std::setw(5) << "ID"
                    <<std::setw(40) << "Project name" << std::endl;
        std::cout << std::string(70,'-') << std::endl;

        int count = 0;
        while (res->next()) {
            std::cout << std::left << std::setw(5) << res->getInt("projectid")
                << std::setw(40) << res->getString("projectname").substr(0,39)
                << std::endl;
            count ++;
        }
        std::cout << std::string(70, '=') << std::endl;
        std::cout << "Total: " << count << " projects" << std::endl;
    }

    void listTasks(Database& db, int activityId) {
        std::string query = R"(
            SELECT t.taskid, t.taskname
            FROM ActivityTasks t
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
        std::cout << "\n" << std::string(70, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(40) << "Task Name"
                  << std::endl;
        std::cout << std::string(70, '-') << std::endl;
        int count = 0;
        while (res->next()) {
            std::cout << std::left << std::setw(5) << res->getInt("taskid")
                      << std::setw(40) << res->getString("taskname").substr(0, 39)
                      << std::endl;
            count++;
        }
        std::cout << std::string(70, '=') << std::endl;
        std::cout << "Total: " << count << " tasks" << std::endl;
    }

    void listSubTasks(Database& db, int taskId) {
        std::string query = R"(
            SELECT t.taskid, t.taskname
            FROM ActivityTasks t
            WHERE t.parenttaskid = ?
        )";

        auto stmt = db.prepareStatement(query);
        stmt->setInt(1, taskId);
        auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
        std::cout << "\n" << std::string(70, '=') << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                  << std::setw(40) << "Task Name"
                  << std::endl;
        std::cout << std::string(70, '-') << std::endl;
        int count = 0;
        while (res->next()) {
            std::cout << std::left << std::setw(5) << res->getInt("taskid")
                      << std::setw(40) << res->getString("taskname").substr(0, 39)
                      << std::endl;
            count++;
        }
        std::cout << std::string(70, '=') << std::endl;
        std::cout << "Total: " << count << " tasks" << std::endl;
    }

} // namespace list
} // namespace commands
