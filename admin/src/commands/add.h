#ifndef ADD_H
#define ADD_H

#include "../data/database.h"
#include "../data/path.h"
#include "../utils/auth.h"
#include "../data/projectdata.h"
#include "../data/activitydata.h"
#include "../data/categorydata.h"
#include "../data/subcategorydata.h"
#include "../data/taskdata.h"
#include "../data/employeedata.h"
#include "../data/artefacttypedata.h"
#include "../data/artefactdata.h"
#include "../data/notedata.h"
#include "../data/fielddata.h"
#include "../data/milestonedata.h"
#include "../data/milestonestepdata.h"
#include "../data/departmentdata.h"
#include "../data/projectteammemberdata.h"
#include "../data/assignmentdata.h"
#include "../data/hoursdata.h"
#include "../data/artefactdataitemdata.h"
#include "../data/artefacttoactivitylinkdata.h"
#include "../data/milestonetoartefactlinkdata.h"

#include <string>

using namespace data;

namespace commands {
// Add an item interactively (prompts user for input)
void add(Database& db, Path& pth);

// Add an item using comma-separated values
void add(Database& db, Path& pth, const std::string& csvValues);

// Function to add a project
int addProject(Database& db, ProjectData& proj);

// Function to add an activity category
int addCategory(Database& db, CategoryData& cat, int projectId);

// Function to add an activity sub-category
int addSubcategory(Database& db, SubCategoryData& subCat, int categoryId);

// Function to add an activity
int addActivity(Database& db, ActivityData& act, int subcategoryId, int projectId = 0);

// Function to add a task
int addTask(Database& db, TaskData& tas, int activityId, int parentTaskId);

// Function to add a new employee
int addEmployee(Database& db, EmployeeData& emp);

// Function to add a new artefact type
int addArtefactType(Database& db, ArtefactTypeData& artTyp);

// Function to add a new artefact
int addArtefact(Database& db, ArtefactData& art, int projectId = 0);

// Function to add a new note
int addNote(Database& db, NoteData& noe, int projectId = 0);

// Function to add a new field
int addField(Database& db, FieldData& fie);

// Function to add a new milestone
int addMilestone(Database& db, MilestoneData& mil);

// Function to add a new milestone step
int addMilestoneStep(Database& db, MilestoneStepData& milStp);

// Function to add a new department
int addDepartment(Database& db, DepartmentData& dept);

// Function to add a new team member
int addTeamMember(Database& db, ProjectTeamMemberData& member);

// Function to add a new assignment
int addAssignment(Database& db, AssignmentData& assignment, int projectId = 0);

// Function to add hours
int addHours(Database& db, HoursData& hours);

// Function to add artefact data item
int addArtefactDataItem(Database& db, ArtefactDataItemData& data);

// Function to add artefact to activity link
int addArtefactToActivityLink(Database& db, ArtefactToActivityLinkData& link);

// Function to add milestone to artefact link
int addMilestoneToArtefactLink(Database& db, MilestoneToArtefactLinkData& link, int projectId = 0);
} // namespace commands

#endif //ADD_H
