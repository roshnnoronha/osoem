#ifndef REMOVE_H
#define REMOVE_H

#include "../data/database.h"
#include "../data/path.h"
#include "../utils/auth.h"
#include <string>

using namespace data;

namespace commands {

// Function providing a common interface to remove an item
void remove (Database& db, Path& pth, std::string param, bool suppressConfirmation = false);

// Function to remove a project given a project id.
void removeProject(Database& db, int projectId);

// Function to remove an activity category given a category id.
void removeCategory(Database& db, int categoryId);

// Function to remove an activity sub-category given a sub-category id.
void removeSubCategory(Database& db, int subCagegoryId);

// Function to remove an activity given an activity id.
void removeActivity(Database& db, int activityId);

// Function to remove a task given a task id.
void removeTask(Database& db, int taskId); 

// Function to remove an employee given an employeeId
void removeEmployee(Database& db, int id);

// Function to remove a note given a note id
void removeNote(Database& db, int noteId);

// Function to remove an artefact type given an artefact type id
void removeArtefactType(Database& db, int artefactTypeId);

// Function to remove an artefact given an artefact id
void removeArtefact(Database& db, int artefactId);

// Function to remove a field given a field id
void removeField(Database& db, int fieldId);

// Function to remove a milestone given a milestone id
void removeMilestone(Database& db, int milestoneId);

// Function to remove a milestone step given a milestone step id
void removeMilestoneStep(Database& db, int milestoneStepId);

// Function to remove a department given a department id
void removeDepartment(Database& db, int departmentId);

// Function to remove a team member given a team member id
void removeTeamMember(Database& db, int teamMemberId);

// Function to remove an assignment given an assignment id
void removeAssignment(Database& db, int assignmentId);

// Function to remove hours given a booking id
void removeHours(Database& db, int bookingId);

// Function to remove an artefact data item given an artefact data id
void removeArtefactDataItem(Database& db, int artefactDataId);

// Function to remove an artefact to activity link given a link id
void removeArtefactToActivityLink(Database& db, int artefactLinkId);

// Function to remove a milestone to artefact link given a link id
void removeMilestoneToArtefactLink(Database& db, int milestoneLinkId);

} // namespace commands


#endif
