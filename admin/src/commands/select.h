#ifndef SELECT_H
#define SELECT_H
#include "../database.h"
#include "../path.h"

namespace commands{
namespace select {   
    

    void selectProject(Database& db, Path& pth, int id) ;


    void selectCategory(Database& db, Path& pth, int id) ;


    void selectSubCategory(Database& db, Path& pth, int id) ;


    void selectActivity(Database& db, Path& pth, int id) ;


    void selectTask(Database& db, Path& pth, int id) ;


    void selectSubTask(Database& db, Path& pth, int id) ;


} //namespace select
} //namespace commands
#endif //SELECT_H
