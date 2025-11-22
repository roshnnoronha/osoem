#ifndef REMOVE_H
#define REMOVE_H

#include "../database.h"

namespace commands {
namespace remove {

    // Function to remove a project given a project id.
    void removeProject(Database& db, int projectId);

    // Function to remove an activity category given a category id.
    void removeCategory(Database& db, int categoryId);

    // Function to remove an activity sub-category given a sub-category id.
    void removeSubCategory(Database& db, int subCagegoryId);

    // Function to remove an activity given an activity id.
    void removeActivity(Database& db, int activityId);

    // Function to remove a task given a task id.
    void removeTask(Database& db, int taskId); 

} // namespace remove
} // namespace commands


#endif
