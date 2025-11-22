#include "select.h"

#include <iostream>
#include <string>

namespace commands {
namespace select {

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
            std::cerr<<"Invalid project.";
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
            std::cerr<<"Invalid category.";
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
            std::cerr<<"Invalid sub-category.";
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
            std::cerr<<"Invalid activity.";
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
            std::cerr<<"Invalid task.";
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
            std::cerr<<"Invalid sub-task.";
        }
    }
} // namespace select
} // namespace commands
