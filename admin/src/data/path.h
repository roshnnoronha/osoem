#ifndef PATH_H
#define PATH_H

#include <deque>
#include <string>
#include <vector>
#include <map>

namespace data {

class Path{
public:
    enum ItemType {
        ROOT,
        // Organization hierarchy
        ORGANIZATION_FOLDER,
        DEPARTMENTS_FOLDER,
        DEPARTMENT,
        EMPLOYEE,
        // Project hierarchy
        PROJECTS_FOLDER,
        PROJECT,
        // Project sub-folders
        TEAM_FOLDER,
        PROJECT_TEAM_MEMBER,
        ACTIVITIES_FOLDER,
        CATEGORY,
        SUBCATEGORY,
        ACTIVITY,
        // Activity sub-folders
        TASKS_FOLDER,
        TASK,
        SUBTASK,
        ASSIGNMENTS_FOLDER,
        ASSIGNMENT,
        HOURS,
        NOTES_FOLDER,
        ACTIVITY_NOTE,
        // Artefact hierarchy
        ARTEFACT_TYPES_FOLDER,
        ARTEFACT_TYPE,
        ARTEFACTS_FOLDER,
        ARTEFACT,
        DATA_FOLDER,
        ARTEFACT_DATA_ITEM,
        ASSOCIATIONS_FOLDER,
        ARTEFACT_TO_ACTIVITY_LINK,
        MILESTONE_TO_ARTEFACT_LINK,
        FIELDS_FOLDER,
        ARTEFACT_FIELD,
        // Milestone hierarchy
        MILESTONES_FOLDER,
        MILESTONE,
        MILESTONE_STEP
    };
    struct Item{
        std::string name;
        int id;
        ItemType type;
    };
    Path();
    Path(std::vector<std::map<std::string,std::string>> rows);

    // Return the string of the current path
    std::string to_string();

    // Push item
    void move_forward(std::string name, int id);

    // Push item with explicit type
    void move_forward(std::string name, int id, ItemType type);

    // Pop item
    void move_back();
    
    // return to the root
    void reset();

    //Get current item type
    ItemType current_type(); 

    // Get current item id
    int current_id();

    // Get current name
    std::string current_name();

    // Get item ID by type (searches path for item of given type)
    // Returns -1 if not found
    int getIdByType(ItemType type);
private:
    std::deque<Item> q;
};

} // namespace data

#endif // PATH_H
