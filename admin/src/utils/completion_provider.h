#ifndef COMPLETION_PROVIDER_H
#define COMPLETION_PROVIDER_H

#include <string>
#include <vector>
#include "../data/database.h"
#include "../data/path.h"

using namespace data;

class CompletionProvider {
public:
    CompletionProvider(Database& db, Path& path);

    // Get completions based on current path context and prefix
    std::vector<std::string> getCompletions(const std::string& prefix);

    // Get completions for a potentially multi-segment path (e.g. ~/projects/MyPro)
    std::vector<std::string> getPathCompletions(const std::string& fullPrefix);

    // Get command completions
    std::vector<std::string> getCommandCompletions(const std::string& prefix);

private:
    Database& db;
    Path& path;

    // Lightweight simulated path item used by getPathCompletions
    struct SimItem {
        Path::ItemType type;
        int id;
    };
    using SimStack = std::vector<SimItem>;

    // Search simStack for the first item of the given type; returns -1 if not found
    int getIdInSim(const SimStack& stack, Path::ItemType type) const;

    // Advance the simulated stack by one path segment (folder name or entity name).
    // Returns false if navigation fails (unknown name, DB miss, leaf node).
    bool navigateSimForward(SimStack& stack, const std::string& segment);

    // Get completions for the last (incomplete) segment in the simulated context
    std::vector<std::string> getCompletionsForSim(const SimStack& stack, const std::string& prefix);

    // Helper to quote names with spaces
    std::string quoteIfNeeded(const std::string& name);

    // Entity-specific completion methods
    std::vector<std::string> getRootCompletions(const std::string& prefix);
    std::vector<std::string> getEmployeeCompletions(const std::string& prefix);
    std::vector<std::string> getProjectCompletions(const std::string& prefix);
    std::vector<std::string> getProjectFolderCompletions(const std::string& prefix);
    std::vector<std::string> getCategoryCompletions(const std::string& prefix, int projectId);
    std::vector<std::string> getSubcategoryCompletions(const std::string& prefix, int categoryId);
    std::vector<std::string> getActivityCompletions(const std::string& prefix, int subcategoryId);
    std::vector<std::string> getActivityFolderCompletions(const std::string& prefix);
    std::vector<std::string> getTaskCompletions(const std::string& prefix, int activityId);
    std::vector<std::string> getSubtaskCompletions(const std::string& prefix, int parentTaskId);
    std::vector<std::string> getNoteCompletions(const std::string& prefix, int activityId);
    std::vector<std::string> getArtefactTypeCompletions(const std::string& prefix, int projectId);
    std::vector<std::string> getArtefactTypeFolderCompletions(const std::string& prefix);
    std::vector<std::string> getArtefactCompletions(const std::string& prefix, int artefactTypeId);
    std::vector<std::string> getFieldCompletions(const std::string& prefix, int artefactTypeId);
    std::vector<std::string> getMilestoneCompletions(const std::string& prefix, int projectId);
    std::vector<std::string> getMilestoneStepCompletions(const std::string& prefix, int milestoneId);

    // New completion methods for updated hierarchy
    std::vector<std::string> getOrganizationFolderCompletions(const std::string& prefix);
    std::vector<std::string> getDepartmentCompletions(const std::string& prefix);
    std::vector<std::string> getDepartmentEmployeeCompletions(const std::string& prefix, int departmentId);
    std::vector<std::string> getTeamMemberCompletions(const std::string& prefix, int projectId);
    std::vector<std::string> getTaskFolderCompletions(const std::string& prefix);
    std::vector<std::string> getAssignmentCompletions(const std::string& prefix, int taskId);
    std::vector<std::string> getHoursCompletions(const std::string& prefix, int assignmentId);
    std::vector<std::string> getArtefactFolderCompletions(const std::string& prefix);
    std::vector<std::string> getArtefactDataItemCompletions(const std::string& prefix, int artefactId);
    std::vector<std::string> getArtefactToActivityLinkCompletions(const std::string& prefix, int artefactId);
    std::vector<std::string> getMilestoneToArtefactLinkCompletions(const std::string& prefix, int artefactLinkId);
};

#endif // COMPLETION_PROVIDER_H
