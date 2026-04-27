#ifndef SET_H
#define SET_H

#include "../data/database.h"
#include "../data/path.h"
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
#include "../utils/auth.h"
#include "../utils/csv_parser.h"
#include "../utils/lookup.h"

#include <string>

using namespace data;

namespace commands {

// Function to provide a common interface to set a value in the database
void set (Database& db, Path& pth, std::string id, std::string csvValues);

// Function to set employee data
void setEmployeeData(Database& db, EmployeeData& emp, int employeeId);

// Function to set project data
void setProjectData(Database& db, ProjectData& proj, int projectId);

// Function to set activity data
void setActivityData(Database& db, ActivityData& act, int activityId);

// Function to set category data
void setCategoryData(Database& db, CategoryData& cat, int categoryId);

// Function to set subcategory data
void setSubcategoryData(Database& db, SubCategoryData& subcat, int subcategoryId);

// Function to set task data
void setTaskData(Database& db, TaskData& task, int taskId);

// Function to set artefact type data
void setArtefactTypeData(Database& db, ArtefactTypeData& artType, int artefactTypeId);

// Function to set artefact data
void setArtefactData(Database& db, ArtefactData& art, int artefactId);

// Function to set note data
void setNoteData(Database& db, NoteData& note, int noteId);

// Function to set field data
void setFieldData(Database& db, FieldData& field, int fieldId);

// Function to set milestone data
void setMilestoneData(Database& db, MilestoneData& milestone, int milestoneId);

// Function to set milestone step data
void setMilestoneStepData(Database& db, MilestoneStepData& step, int milestoneStepId);

// Function to set department data
void setDepartmentData(Database& db, DepartmentData& dept, int departmentId);

// Function to set team member data
void setTeamMemberData(Database& db, ProjectTeamMemberData& member, int teamMemberId);

// Function to set assignment data
void setAssignmentData(Database& db, AssignmentData& assignment, int assignmentId);

// Function to set hours data
void setHoursData(Database& db, HoursData& hours, int bookingId);

// Function to set artefact data item data
void setArtefactDataItemData(Database& db, ArtefactDataItemData& data, int artefactDataId);

// Function to set artefact to activity link data
void setArtefactToActivityLinkData(Database& db, ArtefactToActivityLinkData& link, int artefactLinkId);

// Function to set milestone to artefact link data
void setMilestoneToArtefactLinkData(Database& db, MilestoneToArtefactLinkData& link, int milestoneLinkId);

} // namespace commands
#endif //SET_H
