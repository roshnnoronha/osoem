#ifndef PATH_H
#define PATH_H

#include <deque>
#include <string>

class Path{
public:
    enum ItemType {
        ROOT,
        EMPLOYEES,
        PROJECT,
        // Virtual folders under project
        ACTIVITIES_FOLDER,
        ARTEFACTS_FOLDER,
        MILESTONES_FOLDER,
        // Activity hierarchy
        CATEGORY,
        SUBCATEGORY,
        ACTIVITY,
        TASK,
        ACTIVITY_NOTE,
        // Artefact hierarchy
        ARTEFACT_TYPE,
        ARTEFACT_FIELD,
        ARTEFACT,
        ARTEFACT_DATA,
        // Milestone hierarchy
        MILESTONE,
        MILESTONE_STEP
    };
    struct Item{
        std::string name;
        int id;
        ItemType type;
    };
    Path();

    // Return the string of the current path
    std::string to_string();

    // Push item
    void move_forward(std::string name, int id);

    // Push item with explicit type
    void move_forward(std::string name, int id, ItemType type);

    // Pop item
    void move_back();

    //Get current item type
    ItemType current_type(); 

    // Get current item id
    int current_id();

    // Get child id given a name
    int child_id(std::string name);
private:
    std::deque<Item> q;
};

#endif // PATH_H
