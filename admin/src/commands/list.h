#ifndef LIST_H
#define LIST_H

#include "../data/database.h"
#include "../data/path.h"

using namespace data;

namespace commands {

    // Common interface function for list command
    // showAll: if true, shows full table with all details; if false, shows "(ID) NAME" format
    void list(Database& db, Path& pth, bool showAll = false);

    // List all activities
    void listActivities(Database& db, int projectId, bool showAll = false);

    // List all activity categories
    void listCategories(Database& db, int projectId, bool showAll = false);

    // List all activity sub-categories
    void listSubcategories(Database& db, int categoryId, bool showAll = false);

    // List all tasks
    void listTasks(Database& db, int activityId, bool showAll = false);

    // List all sub-tasks
    void listSubTasks(Database& db, int taskId, bool showAll = false);

    // List all projects
    void listProjects(Database& db, bool showAll = false);

    // List the notes for a given activity
    void listNotes(Database& db, int activityId, bool showAll = false);

    // List the artefact types in a given project
    void listArtefactTypes(Database& db, int projectId, bool showAll = false);

    // List the artefacts of a given artefact type
    void listArtefacts(Database& db, int artefactTypeId, bool showAll = false);

    // List the data fields of a given artefact type
    void listFields(Database& db, int artefactTypeId, bool showAll = false);

    // List the milestones in a given project
    void listMilestones(Database& db, int projectId, bool showAll = false);

    // List the steps in a given milestone
    void listMilestoneSteps(Database& db, int milestoneId, bool showAll = false);

    // List all employees (optionally filtered by department)
    void listEmployees(Database& db, int departmentId = 0, bool showAll = false);

    // List all departments
    void listDepartments(Database& db, bool showAll = false);

    // List team members in a project
    void listTeamMembers(Database& db, int projectId, bool showAll = false);

    // List assignments for a task
    void listAssignments(Database& db, int taskId, bool showAll = false);

    // List hours for an assignment
    void listHours(Database& db, int assignmentId, bool showAll = false);

    // List artefact data items
    void listArtefactDataItems(Database& db, int artefactId, bool showAll = false);

    // List artefact-activity links
    void listArtefactToActivityLinks(Database& db, int artefactId, bool showAll = false);

    // List milestone-artefact links
    void listMilestoneToArtefactLinks(Database& db, int artefactLinkId, bool showAll = false);

} // namespace commands

#endif // LIST_H
