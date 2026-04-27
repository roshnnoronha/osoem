#ifndef IMPORTCSV_H
#define IMPORTCSV_H

#include "../data/database.h"
#include "../data/path.h"
#include "../utils/auth.h"

using namespace data;

namespace commands {
// Function to import data from a csv file `filename`.
// If flag ignoreDuplicates is false the function will throw an exception if the data being entered is already present in the database.
void importcsv (Database& db, Path& pth, std::string filename, bool ignoreDuplicates = false);

// Function to recursively import data from a csv file `filename`.
// Dispatches to the appropriate recursive add function based on the current path context.
void importcsvRecursive(Database& db, Path& pth, std::string filename);

// Function to add new departments recursively
void addDepartmentsRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows);

// Function to add new employees recursively
void addEmployeesRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int departmentId);

// Function to add projects recursively
void addProjectsRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows);

// Function to add activity categories recursively
void addCategoriesRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int projectId);

// Function to add activity sub-categories recursively
void addSubcategoriesRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int categoryId, int projectId);

// Function to add activities recursively
void addActivitiesRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int subcategoryId, int projectId) ;

// Function to add tasks recursively
void addTasksRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int activityId, int parentTaskId, int projectId) ;

// Function to add new notes recursively
void addNotesRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int activityId, int projectId) ;

// Function to add new artefact types recursively
void addArtefactTypesRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int projectId);

// Function to add new artefacts recursively
void addArtefactsRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int artefactTypeId, int projectId);

// Function to add artefact to activity links recursively
void addArtefactToActivityLinksRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int artefactId, int projectId);

// Function to add milestone to artefact links recursively
void addMilestoneToArtefactLinksRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int artefactLinkId, int projectId = 0);

// Function to add new fields recursively
void addFieldsRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int artefactTypeId);

// Function to add artefact data recursively
void addArtefactDataItemsRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int artefactId, int artefactTypeId);

// Function to add new milestones recursively
void addMilestonesRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int projectId);

// Function to add new milestone steps recursively
void addMilestoneStepsRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows , int milestoneId);

// Function to add new team members recursively
void addTeamMembersRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int projectId);

// Function to add new assignments recursively
void addAssignmentsRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int taskId, int projectId);

// Function to add hours recursively
void addHoursRecursive(Database& db, std::vector<std::map<std::string,std::string>>& rows, int assignmentId);





} //namespace commands

#endif  // IMPORTCSV_H
