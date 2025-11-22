#ifndef LIST_H
#define LIST_H

#include "../database.h"

namespace commands {
namespace list {

    // List all projects
    void listProjects(Database& db);

    // List all activity categories
    void listCategories(Database& db, int projectId);

    // List all activity sub-categories
    void listSubcategories(Database& db, int categoryId);

    // List all activities 
    void listActivities(Database& db, int projectId );

    // List all tasks
    void listTasks(Database& db, int activityId) ;

    // List all sub-tasks
    void listSubTasks(Database& db, int taskId) ;

} // namespace list
} // namespace commands

#endif // LIST_H
