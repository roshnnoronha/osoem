#include "path.h"

Path::Path(){
    Item root;
    root.type = ROOT;
    q.push_back(root);
}

std::string Path::to_string(){
    std::string path_string = "";
    for (auto item:q){
       if (item.type == ROOT)
          path_string = "/";
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
       temp.type = PROJECT;
       break;
    case PROJECT:
       temp.type = CATEGORY;
       break;
    case CATEGORY:
       temp.type = SUBCATEGORY;
       break;
    case SUBCATEGORY:
       temp.type = ACTIVITY;
       break;
    case ACTIVITY:
    case TASK:
       temp.type = TASK;
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

Path::ItemType Path::current_type(){
    return q.back().type;
}

int Path::current_id(){
    return q.back().id;
}
