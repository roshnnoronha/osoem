#include "path.h"

namespace data {

Path::Path(){
    Item root;
    root.type = ROOT;
    q.push_back(root);
}

Path::Path(std::vector<std::map<std::string,std::string>> rows) {
    Item root;
    root.type = ROOT;
    q.push_back(root);
    for (const auto& row : rows) {
        Item item;
        item.name = row.count("name") ? row.at("name") : "";
        item.id = row.count("id") ? std::stoi(row.at("id")) : 0;
        item.type = row.count("type") ? static_cast<ItemType>(std::stoi(row.at("type"))) : ROOT;
        q.push_back(item);
    }
}

std::string Path::to_string(){
    std::string path_string = "";
    for (auto item:q){
       if (item.type == ROOT)
          path_string = " ~";
       else 
          path_string = path_string + "/" + item.name; 
    }
    return path_string;
}

void Path::move_forward(std::string name, int id){
    Item temp;
    temp.name = name;
    temp.id = id;
    switch (current_type()){
    case ROOT:
       break;
    // Organization hierarchy
    case ORGANIZATION_FOLDER:
       break;
    case DEPARTMENTS_FOLDER:
       temp.type = DEPARTMENT;
       break;
    case DEPARTMENT:
       temp.type = EMPLOYEE;
       break;
    // Project hierarchy
    case PROJECTS_FOLDER:
       temp.type = PROJECT;
       break;
    case PROJECT:
       break;
    // Team folder
    case TEAM_FOLDER:
       temp.type = PROJECT_TEAM_MEMBER;
       break;
    // Activity hierarchy
    case ACTIVITIES_FOLDER:
       temp.type = CATEGORY;
       break;
    case CATEGORY:
       temp.type = SUBCATEGORY;
       break;
    case SUBCATEGORY:
       temp.type = ACTIVITY;
       break;
    case ACTIVITY:
       break;
    // Task hierarchy
    case TASKS_FOLDER:
       temp.type = TASK;
       break;
    case TASK:
       break;
    case SUBTASK:
       break;
    // Assignments
    case ASSIGNMENTS_FOLDER:
       temp.type = ASSIGNMENT;
       break;
    case ASSIGNMENT:
       temp.type = HOURS;
       break;
    // Notes
    case NOTES_FOLDER:
       temp.type = ACTIVITY_NOTE;
       break;
    // Artefact type hierarchy
    case ARTEFACT_TYPES_FOLDER:
       temp.type = ARTEFACT_TYPE;
       break;
    case ARTEFACT_TYPE:
       break;
    case ARTEFACTS_FOLDER:
       temp.type = ARTEFACT;
       break;
    case ARTEFACT:
       break;
    // Artefact data
    case DATA_FOLDER:
       temp.type = ARTEFACT_DATA_ITEM;
       break;
    // Artefact associations
    case ASSOCIATIONS_FOLDER:
       temp.type = ARTEFACT_TO_ACTIVITY_LINK;
       break;
    case ARTEFACT_TO_ACTIVITY_LINK:
       temp.type = MILESTONE_TO_ARTEFACT_LINK;
       break;
    // Fields
    case FIELDS_FOLDER:
       temp.type = ARTEFACT_FIELD;
       break;
    // Milestones
    case MILESTONES_FOLDER:
       temp.type = MILESTONE;
       break;
    case MILESTONE:
       temp.type = MILESTONE_STEP;
       break;
    default:
       break;
    }
    q.push_back(temp);
}

void Path::move_forward(std::string name, int id, ItemType type){
    Item temp;
    temp.name = name;
    temp.id = id;
    temp.type = type;
    q.push_back(temp);
}

void Path::move_back(){
    if (!q.empty())
        q.pop_back();
}

void Path::reset() {
    while (!q.back().type == ROOT) {
        q.pop_back();
    } 
}

Path::ItemType Path::current_type(){
    return q.back().type;
}

int Path::current_id(){
    return q.back().id;
}

std::string Path::current_name(){
    return q.back().name;
}

int Path::getIdByType(ItemType type) {
    for (const auto& item : q) {
        if (item.type == type) {
            return item.id;
        }
    }
    return -1;
}

} // namespace data
