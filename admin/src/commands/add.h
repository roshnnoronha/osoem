#ifndef ADD_H
#define ADD_H

#include "../database.h"
#include <string>

namespace commands {
namespace add {    
    // Function to add a project
    void addProject(Database& db, const std::string& projectName, const std::string& projectNumber, int projectManagerId);

    // Function to add an activity category
    void addCategory(Database& db, int projectId, const std::string& categoryName);

    // Function to add an activity sub-category
    void addSubcategory(Database& db, int categoryId, const std::string& subCategoryName);

    // Function to add an activity
    void addActivity(Database& db, const std::string& name, int subcategoryId,int managerId, const std::string& startDate,const std::string& endDate, double plannedHours) ;

    // Function to add a task
    void addTask(Database& db, int activityId, const std::string& taskName, int parentTaskId) ;

} // namespace add
} // namespace commands

#endif //ADD_H
