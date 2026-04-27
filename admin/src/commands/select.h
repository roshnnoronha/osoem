#ifndef SELECT_H
#define SELECT_H
#include "../data/database.h"
#include "../data/path.h"
#include <string>

using namespace data;

namespace commands{

    // A common interface to select an item with a given Id or Name
    void select(Database& db, Path& pth, std::string param);

    // Helper function to resolve folder names to indices
    int getFolderIndex(Path::ItemType currentType, const std::string& name);

    // Select a given project (by ID or name)
    void selectProject(Database& db, Path& pth, int id);
    void selectProject(Database& db, Path& pth, const std::string& name);

    // Select a given activity category (by ID or name)
    void selectCategory(Database& db, Path& pth, int id);
    void selectCategory(Database& db, Path& pth, const std::string& name);

    // Select a given activity sub-category (by ID or name)
    void selectSubCategory(Database& db, Path& pth, int id);
    void selectSubCategory(Database& db, Path& pth, const std::string& name);

    // Select a given activity (by ID or name)
    void selectActivity(Database& db, Path& pth, int id);
    void selectActivity(Database& db, Path& pth, const std::string& name);

    // Select a given task (by ID or name)
    void selectTask(Database& db, Path& pth, int id);
    void selectTask(Database& db, Path& pth, const std::string& name);

    // Select a given sub-task (by ID or name)
    void selectSubTask(Database& db, Path& pth, int id);
    void selectSubTask(Database& db, Path& pth, const std::string& name);

    // Select an employee (by ID or name)
    void selectEmployee(Database& db, Path& pth, int id);
    void selectEmployee(Database& db, Path& pth, const std::string& name);

    // Select an activity note (by ID or date)
    void selectNote(Database& db, Path& pth, int id);
    void selectNote(Database& db, Path& pth, const std::string& name);

    // Select an artefact type (by ID or name)
    void selectArtefactType(Database& db, Path& pth, int id);
    void selectArtefactType(Database& db, Path& pth, const std::string& name);

    // Select an artefact (by ID or name)
    void selectArtefact(Database& db, Path& pth, int id);
    void selectArtefact(Database& db, Path& pth, const std::string& name);

    // Select an artefact field (by ID or name)
    void selectField(Database& db, Path& pth, int id);
    void selectField(Database& db, Path& pth, const std::string& name);

    // Select a milestone (by ID or name)
    void selectMilestone(Database& db, Path& pth, int id);
    void selectMilestone(Database& db, Path& pth, const std::string& name);

    // Select a milestone step (by ID or name)
    void selectMilestoneStep(Database& db, Path& pth, int id);
    void selectMilestoneStep(Database& db, Path& pth, const std::string& name);

    // Select a department (by ID or name)
    void selectDepartment(Database& db, Path& pth, int id);
    void selectDepartment(Database& db, Path& pth, const std::string& name);

    // Select a team member (by ID)
    void selectTeamMember(Database& db, Path& pth, int id);

    // Select an assignment (by ID or user name)
    void selectAssignment(Database& db, Path& pth, int id);
    void selectAssignment(Database& db, Path& pth, const std::string& name);

    // Select hours (by ID)
    void selectHours(Database& db, Path& pth, int id);

    // Select an artefact data item (by ID)
    void selectArtefactDataItem(Database& db, Path& pth, int id);

    // Select an artefact to activity link (by ID or name)
    void selectArtefactToActivityLink(Database& db, Path& pth, int id);
    void selectArtefactToActivityLink(Database& db, Path& pth, const std::string& name);

    // Select a milestone to artefact link (by ID)
    void selectMilestoneToArtefactLink(Database& db, Path& pth, int id);

} //namespace commands
#endif //SELECT_H
