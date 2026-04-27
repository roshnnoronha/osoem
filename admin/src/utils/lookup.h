#ifndef UTILS_LOOKUP_H
#define UTILS_LOOKUP_H

#include "../data/database.h"
#include <string>
#include <functional>
#include <vector>

namespace utils {

// Result of a lookup operation
struct LookupResult {
    int id;
    bool found;
    bool multipleMatches;
};

// Lookup functions - return ID by name
LookupResult lookupEmployee(data::Database& db, const std::string& name);
LookupResult lookupDepartment(data::Database& db, const std::string& name);
LookupResult lookupActivity(data::Database& db, const std::string& name);
LookupResult lookupProject(data::Database& db, const std::string& name);

// Scoped lookup functions - require a parent ID for context
LookupResult lookupMilestoneStep(data::Database& db, const std::string& name, int milestoneId);
LookupResult lookupArtefactDataField(data::Database& db, const std::string& name, int artefactTypeId);
LookupResult lookupActivityByProject(data::Database& db, const std::string& name, int projectId);
LookupResult lookupActivityByNames(data::Database& db, const std::string& activityName, int projectId, const std::string& categoryName, const std::string& subcategoryName);
LookupResult lookupMilestoneStepByProject(data::Database& db, const std::string& name, int projectId);
LookupResult lookupCategory(data::Database& db, const std::string& name, int projectId);
LookupResult lookupSubcategory(data::Database& db, const std::string& name, int categoryId);
LookupResult lookupActivityBySubcategory(data::Database& db, const std::string& name, int subcategoryId);
LookupResult lookupTaskByParent(data::Database& db, const std::string& name, int parentTaskId);
LookupResult lookupTaskByActivity(data::Database& db, const std::string& name, int activityId);
LookupResult lookupTaskByNames(data::Database& db, const std::string& taskName, int projectId,
                               const std::string& activityName = "",
                               const std::string& subcategoryName = "",
                               const std::string& categoryName = "");
LookupResult lookupNote(data::Database& db, const std::string& notedate, int activityId);
LookupResult lookupArtefactType(data::Database& db, const std::string& name, int projectId);
LookupResult lookupArtefact(data::Database& db, const std::string& name, int artefactTypeId);
LookupResult lookupMilestone(data::Database& db, const std::string& name, int projectId);
LookupResult lookupAssignment(data::Database& db, const std::string& name, int taskId);
LookupResult lookupArtefactActivityLink(data::Database& db, const std::string& name, int artefactId);
LookupResult lookupArtefactDataItem(data::Database& db, const std::string& fieldTitle, int artefactId);
LookupResult lookupTeamMember(data::Database& db, const std::string& name, int projectId);
LookupResult lookupMilestoneToArtefactLink(data::Database& db, const std::string& name, int artefactLinkId);
LookupResult lookupHours(data::Database& db, const std::string& bookedDate, int assignmentId);

// Completion query functions — return candidate names matching prefix (for tab completion)
std::vector<std::string> completionTeamMembers(data::Database& db, const std::string& prefix, int projectId);
std::vector<std::string> completionEmployees(data::Database& db, const std::string& prefix);
std::vector<std::string> completionDepartments(data::Database& db, const std::string& prefix);
std::vector<std::string> completionActivitiesByProject(data::Database& db, const std::string& prefix, int projectId);
std::vector<std::string> completionMilestoneStepsByProject(data::Database& db, const std::string& prefix, int projectId);

// Helper function: prompts for ID or name and resolves to ID
// Returns the resolved ID, or throws ValidationError on failure
// If completionFn is provided, tab completion is available at the prompt.
int promptIdOrName(data::Database& db, const std::string& prompt,
                   std::function<LookupResult(data::Database&, const std::string&)> lookupFn,
                   std::function<std::vector<std::string>(const std::string&)> completionFn = nullptr);

// Scoped version: requires an additional context ID
int promptIdOrNameScoped(data::Database& db, const std::string& prompt, int scopeId,
                         std::function<LookupResult(data::Database&, const std::string&, int)> lookupFn,
                         std::function<std::vector<std::string>(const std::string&)> completionFn = nullptr);

// Helper function for CSV import: resolves value to ID without prompting
// If value is numeric, returns it directly; otherwise performs lookup
// Returns resolved ID, or throws ValidationError on failure
int resolveIdOrName(data::Database& db, const std::string& value,
                    std::function<LookupResult(data::Database&, const std::string&)> lookupFn);

// Scoped version for CSV import: requires a parent ID for context
int resolveIdOrNameScoped(data::Database& db, const std::string& value, int scopeId,
                          std::function<LookupResult(data::Database&, const std::string&, int)> lookupFn);

} // namespace utils

#endif // UTILS_LOOKUP_H
