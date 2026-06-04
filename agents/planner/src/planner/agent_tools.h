#ifndef AGENT_TOOLS_H
#define AGENT_TOOLS_H

#include "openai_client.h"
#include "../data/database.h"

#include <string>
#include <string_view>

namespace planner {

class AgentTools {
public:
    AgentTools(data::Database& db);
    std::vector<Tool> getTools();
    std::string callTool(const ToolCall& toolCall);
    void setWorkingProject(int projectId);
    std::vector<std::pair<int, std::string>> getProjectList();

private:
    data::Database& db_;
    int workingProjectID_ = 0;

    // Read tools
    std::string getWorkingProject();
    std::string getEmployees();
    std::string getDepartments();
    std::string getTeamMembers();
    std::string getEmployeeTimeSummary(int employeeId, int projectId=0);
    std::string getProjects();
    std::string getActivities(int projectId);
    std::string getTasks(int activityId);
    std::string getTaskAssignments(int employeeId, int taskId);
    std::string getEmployeeTaskTimeSummary(int employeeId, int taskId);

    // Create tools
    std::string addTeamMember(int employeeId, int role);
    std::string createActivityCategory(const std::string& categoryName);
    std::string createActivitySubcategory(int categoryId, const std::string& subcategoryName);
    std::string createActivity(int subcategoryId, const std::string& name, int managerId,
        const std::string& description, const std::string& plannedStart,
        const std::string& plannedFinish, double plannedHours);
    std::string createTask(int activityId, const std::string& name, const std::string& description,
        int parentTaskId, int departmentId);

    // Update activity tools
    std::string updateActivityDescription(int activityId, const std::string& description);
    std::string updateActivityManager(int activityId, int managerId);
    std::string updateActivityPlannedStart(int activityId, const std::string& date);
    std::string updateActivityPlannedFinish(int activityId, const std::string& date);
    std::string updateActivityPlannedHours(int activityId, double hours);
    std::string updateActivityForecastStart(int activityId, const std::string& date);
    std::string updateActivityForecastFinish(int activityId, const std::string& date);
    std::string updateActivityForecastHours(int activityId, double hours);

    // Update task tools
    std::string updateTaskDescription(int taskId, const std::string& description);
    std::string updateTaskDepartment(int taskId, int departmentId);

    // Update team member tools
    std::string updateTeamMemberRole(int teamMemberId, int role);

};

} // namespace planner


#endif

