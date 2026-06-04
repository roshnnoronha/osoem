#include "agent_tools.h"
#include <sstream>
#include <stdexcept>

namespace planner {

AgentTools::AgentTools(data::Database& db): db_(db) {
}

void AgentTools::setWorkingProject(int projectId) {
    workingProjectID_ = projectId;
}

std::vector<Tool> AgentTools::getTools() {
    std::vector<Tool> toolList;
    
    //define tools 
    toolList.push_back({
        "get_employees",
        "Returns a list of the current employees and their IDs.",
        {}
    });
    toolList.push_back({
        "get_employee_time_summary",
        "Returns the time booked by an employee in a given project",
        {
            {"employee_id", "integer", "The employee ID for the employee for which the details are required.",{}, true},
            {"project_id", "integer", "The project ID for which employees hours details are required.",{}, true}
        }
    }); 
    toolList.push_back({
        "get_projects",
        "Returns a list of projects and their IDs.",
        {}
    });
    toolList.push_back({
        "get_activities_in_project",
        "Returns a list of activities in a given project, including their IDs, category, subcategory, manager, planned dates, and planned/actual hours.",
        {
            {"project_id", "integer", "The project ID for which to list activities.", {}, true}
        }
    });
    toolList.push_back({
        "get_tasks_in_activity",
        "Returns a list of tasks in a given activity, including their IDs, names, descriptions, parent task, and assigned department.",
        {
            {"activity_id", "integer", "The activity ID for which to list tasks.", {}, true}
        }
    });
    toolList.push_back({
        "create_activity_category",
        "Creates a new activity category in the working project.",
        {
            {"category_name", "string", "The name of the new activity category.", {}, true}
        }
    });
    toolList.push_back({
        "create_activity_subcategory",
        "Creates a new activity subcategory under an existing category in the working project.",
        {
            {"category_id", "integer", "The ID of the parent activity category.", {}, true},
            {"subcategory_name", "string", "The name of the new activity subcategory.", {}, true}
        }
    });
    toolList.push_back({
        "create_activity",
        "Creates a new activity in the working project under a given subcategory.",
        {
            {"subcategory_id", "integer", "The ID of the subcategory for this activity.", {}, true},
            {"activity_name", "string", "The name of the new activity.", {}, true},
            {"manager_id", "integer", "The employee ID of the activity manager.", {}, true},
            {"description", "string", "Optional description of the activity.", {}, false},
            {"planned_start", "string", "Optional planned start date (YYYY-MM-DD).", {}, false},
            {"planned_finish", "string", "Optional planned finish date (YYYY-MM-DD).", {}, false},
            {"planned_hours", "number", "Optional planned hours for the activity.", {}, false}
        }
    });
    toolList.push_back({
        "create_task",
        "Creates a new task within an activity in the working project, optionally nested under a parent task.",
        {
            {"activity_id", "integer", "The ID of the activity this task belongs to.", {}, true},
            {"task_name", "string", "The name of the new task.", {}, true},
            {"description", "string", "Optional description of the task.", {}, false},
            {"parent_task_id", "integer", "Optional ID of the parent task (omit or 0 for a root task).", {}, false},
            {"department_id", "integer", "Optional ID of the department responsible for this task.", {}, false}
        }
    });
    toolList.push_back({
        "update_activity_description",
        "Updates the description of an activity.",
        {
            {"activity_id", "integer", "The ID of the activity to update.", {}, true},
            {"description", "string", "The new description.", {}, true}
        }
    });
    toolList.push_back({
        "update_activity_manager",
        "Updates the manager of an activity.",
        {
            {"activity_id", "integer", "The ID of the activity to update.", {}, true},
            {"manager_id", "integer", "The employee ID of the new activity manager.", {}, true}
        }
    });
    toolList.push_back({
        "update_activity_planned_start",
        "Updates the planned start date of an activity.",
        {
            {"activity_id", "integer", "The ID of the activity to update.", {}, true},
            {"date", "string", "The new planned start date (YYYY-MM-DD).", {}, true}
        }
    });
    toolList.push_back({
        "update_activity_planned_finish",
        "Updates the planned finish date of an activity.",
        {
            {"activity_id", "integer", "The ID of the activity to update.", {}, true},
            {"date", "string", "The new planned finish date (YYYY-MM-DD).", {}, true}
        }
    });
    toolList.push_back({
        "update_activity_planned_hours",
        "Updates the planned hours of an activity.",
        {
            {"activity_id", "integer", "The ID of the activity to update.", {}, true},
            {"hours", "number", "The new planned hours.", {}, true}
        }
    });
    toolList.push_back({
        "update_activity_forecast_start",
        "Updates the forecast start date of an activity.",
        {
            {"activity_id", "integer", "The ID of the activity to update.", {}, true},
            {"date", "string", "The new forecast start date (YYYY-MM-DD).", {}, true}
        }
    });
    toolList.push_back({
        "update_activity_forecast_finish",
        "Updates the forecast finish date of an activity.",
        {
            {"activity_id", "integer", "The ID of the activity to update.", {}, true},
            {"date", "string", "The new forecast finish date (YYYY-MM-DD).", {}, true}
        }
    });
    toolList.push_back({
        "update_activity_forecast_hours",
        "Updates the forecast hours of an activity.",
        {
            {"activity_id", "integer", "The ID of the activity to update.", {}, true},
            {"hours", "number", "The new forecast hours.", {}, true}
        }
    });
    toolList.push_back({
        "update_task_description",
        "Updates the description of a task in the working project.",
        {
            {"task_id", "integer", "The ID of the task to update.", {}, true},
            {"description", "string", "The new description.", {}, true}
        }
    });
    toolList.push_back({
        "update_task_department",
        "Updates the department of a task in the working project.",
        {
            {"task_id", "integer", "The ID of the task to update.", {}, true},
            {"department_id", "integer", "The ID of the department to assign to this task.", {}, true}
        }
    });
    toolList.push_back({
        "get_working_project",
        "Returns the current working project.",
        {}
    });
    toolList.push_back({
        "get_task_assignments",
        "Returns the task assignments of an employee in a particular task.",
        {
            {"employee_id", "integer", "The employee ID.", {}, true},
            {"task_id", "integer", "The task ID.", {}, true}
        }
    });
    toolList.push_back({
        "get_employee_task_time_summary",
        "Returns the time booked by an employee in a task.",
        {
            {"employee_id", "integer", "The employee ID.", {}, true},
            {"task_id", "integer", "The task ID.", {}, true}
        }
    });
    toolList.push_back({
        "get_team_members",
        "Returns the list of team members in the working project, including their employee IDs, names, departments, and roles.",
        {}
    });
    toolList.push_back({
        "add_team_member",
        "Adds an employee as a team member to the working project.",
        {
            {"employee_id", "integer", "The employee ID to add as a team member.", {}, true},
            {"role", "integer", "Optional role identifier for the team member.(role: 0=Member, 1=Lead, 2=Manager)", {}, false}
        }
    });
    toolList.push_back({
        "get_departments",
        "Returns a list of all departments and their IDs.",
        {}
    });
    toolList.push_back({
        "update_team_member_role",
        "Updates the role of a team member in the working project.",
        {
            {"team_member_id", "integer", "The team member ID.", {}, true},
            {"role", "integer", "The new role. (0=Member, 1=Lead, 2=Manager)", {}, true}
        }
    });
    return toolList;
}

std::string AgentTools::callTool(const ToolCall& toolCall) {
    if (toolCall.name == "get_employees") {
        std::cout << "Fetching the list of employees ...\n";
        return getEmployees();
    }
    if (toolCall.name == "get_employee_time_summary") {
        std::cout << "Fetching employee time booking data ...\n";
        int employeeId = 0, projectId = 0;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "employee_id") employeeId = std::stoi(arg.value);
            else if (arg.name == "project_id") projectId = std::stoi(arg.value);
        }
        return getEmployeeTimeSummary(employeeId, projectId);
    }
    if (toolCall.name == "get_projects") {
        std::cout << "Fetching the list of projects ...\n";
        return getProjects();
    }
    if (toolCall.name == "get_activities_in_project") {
        std::cout << "Fetching activities for project ...\n";
        int projectId = 0;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "project_id") projectId = std::stoi(arg.value);
        }
        return getActivities(projectId);
    }
    if (toolCall.name == "get_tasks_in_activity") {
        std::cout << "Fetching tasks for activity ...\n";
        int activityId = 0;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "activity_id") activityId = std::stoi(arg.value);
        }
        return getTasks(activityId);
    }
    if (toolCall.name == "create_activity_category") {
        std::cout << "Creating activity category ...\n";
        std::string categoryName;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "category_name") categoryName = arg.value;
        }
        return createActivityCategory(categoryName);
    }
    if (toolCall.name == "create_activity_subcategory") {
        std::cout << "Creating activity subcategory ...\n";
        int categoryId = 0;
        std::string subcategoryName;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "category_id") categoryId = std::stoi(arg.value);
            else if (arg.name == "subcategory_name") subcategoryName = arg.value;
        }
        return createActivitySubcategory(categoryId, subcategoryName);
    }
    if (toolCall.name == "create_activity") {
        std::cout << "Creating activity ...\n";
        int subcategoryId = 0, managerId = 0;
        std::string name, description, plannedStart, plannedFinish;
        double plannedHours = -1.0;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "subcategory_id") subcategoryId = std::stoi(arg.value);
            else if (arg.name == "activity_name") name = arg.value;
            else if (arg.name == "manager_id") managerId = std::stoi(arg.value);
            else if (arg.name == "description") description = arg.value;
            else if (arg.name == "planned_start") plannedStart = arg.value;
            else if (arg.name == "planned_finish") plannedFinish = arg.value;
            else if (arg.name == "planned_hours") plannedHours = std::stod(arg.value);
        }
        return createActivity(subcategoryId, name, managerId, description, plannedStart, plannedFinish, plannedHours);
    }
    if (toolCall.name == "create_task") {
        std::cout << "Creating task ...\n";
        int activityId = 0, parentTaskId = 0, departmentId = 0;
        std::string name, description;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "activity_id") activityId = std::stoi(arg.value);
            else if (arg.name == "task_name") name = arg.value;
            else if (arg.name == "description") description = arg.value;
            else if (arg.name == "parent_task_id") parentTaskId = std::stoi(arg.value);
            else if (arg.name == "department_id") departmentId = std::stoi(arg.value);
        }
        return createTask(activityId, name, description, parentTaskId, departmentId);
    }
    if (toolCall.name == "update_activity_description") {
        std::cout << "Updating activity description ...\n";
        int activityId = 0;
        std::string description;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "activity_id") activityId = std::stoi(arg.value);
            else if (arg.name == "description") description = arg.value;
        }
        return updateActivityDescription(activityId, description);
    }
    if (toolCall.name == "update_activity_manager") {
        std::cout << "Updating activity manager ...\n";
        int activityId = 0, managerId = 0;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "activity_id") activityId = std::stoi(arg.value);
            else if (arg.name == "manager_id") managerId = std::stoi(arg.value);
        }
        return updateActivityManager(activityId, managerId);
    }
    if (toolCall.name == "update_activity_planned_start") {
        std::cout << "Updating activity planned start date ...\n";
        int activityId = 0;
        std::string date;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "activity_id") activityId = std::stoi(arg.value);
            else if (arg.name == "date") date = arg.value;
        }
        return updateActivityPlannedStart(activityId, date);
    }
    if (toolCall.name == "update_activity_planned_finish") {
        std::cout << "Updating activity planned finish date ...\n";
        int activityId = 0;
        std::string date;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "activity_id") activityId = std::stoi(arg.value);
            else if (arg.name == "date") date = arg.value;
        }
        return updateActivityPlannedFinish(activityId, date);
    }
    if (toolCall.name == "update_activity_planned_hours") {
        std::cout << "Updating activity planned hours ...\n";
        int activityId = 0;
        double hours = 0.0;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "activity_id") activityId = std::stoi(arg.value);
            else if (arg.name == "hours") hours = std::stod(arg.value);
        }
        return updateActivityPlannedHours(activityId, hours);
    }
    if (toolCall.name == "update_activity_forecast_start") {
        std::cout << "Updating activity forecast start date ...\n";
        int activityId = 0;
        std::string date;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "activity_id") activityId = std::stoi(arg.value);
            else if (arg.name == "date") date = arg.value;
        }
        return updateActivityForecastStart(activityId, date);
    }
    if (toolCall.name == "update_activity_forecast_finish") {
        std::cout << "Updating activity forecast finish date ...\n";
        int activityId = 0;
        std::string date;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "activity_id") activityId = std::stoi(arg.value);
            else if (arg.name == "date") date = arg.value;
        }
        return updateActivityForecastFinish(activityId, date);
    }
    if (toolCall.name == "update_activity_forecast_hours") {
        std::cout << "Updating activity forecast hours ...\n";
        int activityId = 0;
        double hours = 0.0;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "activity_id") activityId = std::stoi(arg.value);
            else if (arg.name == "hours") hours = std::stod(arg.value);
        }
        return updateActivityForecastHours(activityId, hours);
    }
    if (toolCall.name == "update_task_description") {
        std::cout << "Updating task description ...\n";
        int taskId = 0;
        std::string description;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "task_id") taskId = std::stoi(arg.value);
            else if (arg.name == "description") description = arg.value;
        }
        return updateTaskDescription(taskId, description);
    }
    if (toolCall.name == "update_task_department") {
        std::cout << "Updating task department ...\n";
        int taskId = 0, departmentId = 0;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "task_id") taskId = std::stoi(arg.value);
            else if (arg.name == "department_id") departmentId = std::stoi(arg.value);
        }
        return updateTaskDepartment(taskId, departmentId);
    }
    if (toolCall.name == "get_working_project") {
        std::cout << "Fetching working project ...\n";
        return getWorkingProject();
    }
    if (toolCall.name == "get_task_assignments") {
        std::cout << "Fetching task assignments ...\n";
        int employeeId = 0, taskId = 0;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "employee_id") employeeId = std::stoi(arg.value);
            else if (arg.name == "task_id") taskId = std::stoi(arg.value);
        }
        return getTaskAssignments(employeeId, taskId);
    }
    if (toolCall.name == "get_employee_task_time_summary") {
        std::cout << "Fetching employee task time booking data ...\n";
        int employeeId = 0, taskId = 0;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "employee_id") employeeId = std::stoi(arg.value);
            else if (arg.name == "task_id") taskId = std::stoi(arg.value);
        }
        return getEmployeeTaskTimeSummary(employeeId, taskId);
    }
    if (toolCall.name == "get_team_members") {
        std::cout << "Fetching team members ...\n";
        return getTeamMembers();
    }
    if (toolCall.name == "add_team_member") {
        std::cout << "Adding team member ...\n";
        int employeeId = 0, role = 0;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "employee_id") employeeId = std::stoi(arg.value);
            else if (arg.name == "role") role = std::stoi(arg.value);
        }
        return addTeamMember(employeeId, role);
    }
    if (toolCall.name == "get_departments") {
        std::cout << "Fetching departments ...\n";
        return getDepartments();
    }
    if (toolCall.name == "update_team_member_role") {
        std::cout << "Updating team member role ...\n";
        int teamMemberId = 0, role = 0;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "team_member_id") teamMemberId = std::stoi(arg.value);
            else if (arg.name == "role") role = std::stoi(arg.value);
        }
        return updateTeamMemberRole(teamMemberId, role);
    }
    throw std::runtime_error("Unknown tool: " + toolCall.name);
}

std::string AgentTools::getEmployees() {
    auto res = db_.executeQuery(
        "SELECT e.employeeid, CONCAT(e.firstname, ' ', e.lastname) AS name, "
        "d.departmentname "
        "FROM Employees e "
        "JOIN Departments d ON e.departmentid = d.departmentid "
        "ORDER BY d.departmentname, e.lastname, e.firstname"
    );
    std::ostringstream ss;
    ss << "## Employee List\n";
    ss << "| Employee ID | Employee Name | Department |\n";
    ss << "| --- | --- | --- |\n";
    while (res->next()) {
        ss << "| " << res->getInt("employeeid") << " | " << res->getString("name") << " | " << res->getString("departmentname") << " | \n";
    }
    return ss.str();
}

std::string AgentTools::getEmployeeTimeSummary(int employeeId, int projectId) {
    std::string query = "SELECT e.employeeid, p.projectname, COALESCE(SUM(h.hours), 0) AS hrs "
            "FROM Employees e "
            "JOIN ActivityTaskAssignments ata ON ata.userid = e.employeeid "
            "JOIN Hours h ON h.assignmentid = ata.assignmentid "
            "JOIN ActivityTasks atsk ON ata.taskid = atsk.taskid "
            "JOIN Activities a ON atsk.activityid = a.activityid "
            "JOIN ActivitySubcategories acs ON a.subcategoryid = acs.subcategoryid "
            "JOIN ActivityCategories ac ON acs.categoryid = ac.categoryid "
            "JOIN Projects p ON ac.projectid = p.projectid "
            "WHERE e.employeeid = ? ";
    if (projectId != 0) query += "AND p.projectid = ? ";
    query += "GROUP BY e.employeeid, p.projectid ";
    auto stmt = db_.prepareStatement (query);
    stmt->setInt(1,employeeId);
    if (projectId != 0) stmt->setInt(2, projectId); 
    auto timeRes = stmt->executeQuery();

    std::ostringstream ss;
    ss << "## Employee Time Booking Summary \n";
    ss << "| Employee ID | Project Name | Hours Booked | \n";
    ss << "| --- | --- | --- | \n";

    while (timeRes->next()) {
        ss << "| " << timeRes->getInt("employeeid") << " | " << timeRes->getString("projectname") << " | " << timeRes->getDouble("hrs") << " | \n";
    }
    return ss.str();
}

std::string AgentTools::getProjects() {
    auto res = db_.executeQuery(
        "SELECT projectid, projectno, projectname FROM Projects ORDER BY projectno"
    );
    std::ostringstream ss;
    ss << "## Project List \n";
    ss << "| Project ID | Project Number | Project Name | \n";
    ss << "| --- | --- | --- | \n";
    while (res->next()) {
        ss << "| " << res->getInt("projectid") << " | "  << res->getString("projectno") << " | " << res->getString("projectname") << " | \n";
    }
    return ss.str();
}
std::vector<std::pair<int, std::string>> AgentTools::getProjectList() {
    auto res = db_.executeQuery(
        "SELECT projectid, projectname FROM Projects ORDER BY projectno"
    );
    std::vector<std::pair<int, std::string>> projects;
    while (res->next())
        projects.emplace_back(res->getInt("projectid"), res->getString("projectname"));
    return projects;
}

std::string AgentTools::getActivities(int projectId) {
    auto stmt = db_.prepareStatement(
        "SELECT a.activityid, a.activityname, "
        "COALESCE(a.activitydescription, '') AS activitydescription, "
        "ac.categoryname, acs.subcategoryname, "
        "CONCAT(e.firstname, ' ', e.lastname) AS managername, "
        "COALESCE(DATE_FORMAT(a.plannedstart, '%Y-%m-%d'), '') AS plannedstart, "
        "COALESCE(DATE_FORMAT(a.plannedfinish, '%Y-%m-%d'), '') AS plannedfinish, "
        "COALESCE(a.plannedhours, 0) AS plannedhours, "
        "COALESCE(SUM(h.hours), 0) AS actualhours "
        "FROM Activities a "
        "JOIN ActivitySubcategories acs ON a.subcategoryid = acs.subcategoryid "
        "JOIN ActivityCategories ac ON acs.categoryid = ac.categoryid "
        "JOIN Employees e ON a.activitymanagerid = e.employeeid "
        "LEFT JOIN ActivityTasks atsk ON atsk.activityid = a.activityid "
        "LEFT JOIN ActivityTaskAssignments ata ON ata.taskid = atsk.taskid "
        "LEFT JOIN Hours h ON h.assignmentid = ata.assignmentid "
        "WHERE ac.projectid = ? "
        "GROUP BY a.activityid, a.activityname, a.activitydescription, "
        "ac.categoryname, acs.subcategoryname, managername, "
        "a.plannedstart, a.plannedfinish, a.plannedhours "
        "ORDER BY ac.categoryname, acs.subcategoryname, a.activityname"
    );
    stmt->setInt(1, projectId);
    auto res = stmt->executeQuery();

    std::ostringstream ss;
    ss << "## Activities in Project\n";
    ss << "| Activity ID | Activity Name | Category | Subcategory | Manager | Planned Start | Planned Finish | Planned Hours | Actual Hours |\n";
    ss << "| --- | --- | --- | --- | --- | --- | --- | --- | --- |\n";
    while (res->next()) {
        ss << "| " << res->getInt("activityid")
           << " | " << res->getString("activityname")
           << " | " << res->getString("categoryname")
           << " | " << res->getString("subcategoryname")
           << " | " << res->getString("managername")
           << " | " << res->getString("plannedstart")
           << " | " << res->getString("plannedfinish")
           << " | " << res->getDouble("plannedhours")
           << " | " << res->getDouble("actualhours")
           << " |\n";
    }
    return ss.str();
}

std::string AgentTools::getTasks(int activityId) {
    auto stmt = db_.prepareStatement(
        "SELECT t.taskid, t.taskname, "
        "COALESCE(t.taskdescription, '') AS taskdescription, "
        "COALESCE(t.parenttaskid, 0) AS parenttaskid, "
        "COALESCE(d.departmentname, '') AS departmentname "
        "FROM ActivityTasks t "
        "LEFT JOIN Departments d ON t.departmentid = d.departmentid "
        "WHERE t.activityid = ? "
        "ORDER BY t.taskid"
    );
    stmt->setInt(1, activityId);
    auto res = stmt->executeQuery();

    std::ostringstream ss;
    ss << "## Tasks in Activity\n";
    ss << "| Task ID | Task Name | Description | Parent Task ID | Department |\n";
    ss << "| --- | --- | --- | --- | --- |\n";
    while (res->next()) {
        ss << "| " << res->getInt("taskid")
           << " | " << res->getString("taskname")
           << " | " << res->getString("taskdescription")
           << " | " << res->getInt("parenttaskid")
           << " | " << res->getString("departmentname")
           << " |\n";
    }
    return ss.str();
}

std::string AgentTools::createActivityCategory(const std::string& categoryName) {
    if (workingProjectID_ == 0) return "Error: no working project set.";
    auto stmt = db_.prepareStatement(
        "INSERT INTO ActivityCategories (projectid, categoryname) VALUES (?, ?)"
    );
    stmt->setInt(1, workingProjectID_);
    stmt->setString(2, categoryName);
    stmt->executeUpdate();
    auto res = db_.executeQuery("SELECT LAST_INSERT_ID() AS id");
    res->next();
    std::ostringstream ss;
    ss << "Activity category created. Category ID: " << res->getInt("id") << ", Name: " << categoryName;
    return ss.str();
}

std::string AgentTools::createActivitySubcategory(int categoryId, const std::string& subcategoryName) {
    if (workingProjectID_ == 0) return "Error: no working project set.";
    auto check = db_.prepareStatement(
        "SELECT categoryid FROM ActivityCategories WHERE categoryid = ? AND projectid = ?"
    );
    check->setInt(1, categoryId);
    check->setInt(2, workingProjectID_);
    auto checkRes = check->executeQuery();
    if (!checkRes->next()) return "Error: category not found in the working project.";

    auto stmt = db_.prepareStatement(
        "INSERT INTO ActivitySubcategories (categoryid, subcategoryname) VALUES (?, ?)"
    );
    stmt->setInt(1, categoryId);
    stmt->setString(2, subcategoryName);
    stmt->executeUpdate();
    auto res = db_.executeQuery("SELECT LAST_INSERT_ID() AS id");
    res->next();
    std::ostringstream ss;
    ss << "Activity subcategory created. Subcategory ID: " << res->getInt("id") << ", Name: " << subcategoryName;
    return ss.str();
}

std::string AgentTools::createActivity(int subcategoryId, const std::string& name, int managerId,
    const std::string& description, const std::string& plannedStart,
    const std::string& plannedFinish, double plannedHours) {
    if (workingProjectID_ == 0) return "Error: no working project set.";

    std::string cols = "INSERT INTO Activities (subcategoryid, activityname, activitymanagerid";
    std::string vals = ") VALUES (?, ?, ?";
    bool hasDesc = !description.empty();
    bool hasStart = !plannedStart.empty();
    bool hasFinish = !plannedFinish.empty();
    bool hasHours = plannedHours >= 0.0;
    if (hasDesc)   { cols += ", activitydescription"; vals += ", ?"; }
    if (hasStart)  { cols += ", plannedstart";         vals += ", ?"; }
    if (hasFinish) { cols += ", plannedfinish";        vals += ", ?"; }
    if (hasHours)  { cols += ", plannedhours";         vals += ", ?"; }

    auto stmt = db_.prepareStatement(cols + vals + ")");
    int p = 1;
    stmt->setInt(p++, subcategoryId);
    stmt->setString(p++, name);
    stmt->setInt(p++, managerId);
    if (hasDesc)   stmt->setString(p++, description);
    if (hasStart)  stmt->setString(p++, plannedStart);
    if (hasFinish) stmt->setString(p++, plannedFinish);
    if (hasHours)  stmt->setDouble(p++, plannedHours);
    stmt->executeUpdate();

    auto res = db_.executeQuery("SELECT LAST_INSERT_ID() AS id");
    res->next();
    std::ostringstream ss;
    ss << "Activity created. Activity ID: " << res->getInt("id") << ", Name: " << name;
    return ss.str();
}

std::string AgentTools::createTask(int activityId, const std::string& name, const std::string& description,
    int parentTaskId, int departmentId) {
    if (workingProjectID_ == 0) return "Error: no working project set.";

    std::string cols = "INSERT INTO ActivityTasks (activityid, taskname";
    std::string vals = ") VALUES (?, ?";
    bool hasDesc   = !description.empty();
    bool hasParent = parentTaskId != 0;
    bool hasDept   = departmentId != 0;
    if (hasDesc)   { cols += ", taskdescription"; vals += ", ?"; }
    if (hasParent) { cols += ", parenttaskid";    vals += ", ?"; }
    if (hasDept)   { cols += ", departmentid";    vals += ", ?"; }

    auto stmt = db_.prepareStatement(cols + vals + ")");
    int p = 1;
    stmt->setInt(p++, activityId);
    stmt->setString(p++, name);
    if (hasDesc)   stmt->setString(p++, description);
    if (hasParent) stmt->setInt(p++, parentTaskId);
    if (hasDept)   stmt->setInt(p++, departmentId);
    stmt->executeUpdate();

    auto res = db_.executeQuery("SELECT LAST_INSERT_ID() AS id");
    res->next();
    std::ostringstream ss;
    ss << "Task created. Task ID: " << res->getInt("id") << ", Name: " << name;
    return ss.str();
}

std::string AgentTools::updateActivityDescription(int activityId, const std::string& description) {
    auto stmt = db_.prepareStatement(
        "UPDATE Activities a "
        "JOIN ActivitySubcategories acs ON a.subcategoryid = acs.subcategoryid "
        "JOIN ActivityCategories ac ON acs.categoryid = ac.categoryid "
        "SET a.activitydescription = ? "
        "WHERE a.activityid = ? AND ac.projectid = ?"
    );
    stmt->setString(1, description);
    stmt->setInt(2, activityId);
    stmt->setInt(3, workingProjectID_);
    int rows = stmt->executeUpdate();
    if (rows == 0) return "Error: activity ID " + std::to_string(activityId) + " not found in the working project.";
    return "Activity " + std::to_string(activityId) + " description updated.";
}

std::string AgentTools::updateActivityManager(int activityId, int managerId) {
    auto stmt = db_.prepareStatement(
        "UPDATE Activities a "
        "JOIN ActivitySubcategories acs ON a.subcategoryid = acs.subcategoryid "
        "JOIN ActivityCategories ac ON acs.categoryid = ac.categoryid "
        "SET a.activitymanagerid = ? "
        "WHERE a.activityid = ? AND ac.projectid = ?"
    );
    stmt->setInt(1, managerId);
    stmt->setInt(2, activityId);
    stmt->setInt(3, workingProjectID_);
    int rows = stmt->executeUpdate();
    if (rows == 0) return "Error: activity ID " + std::to_string(activityId) + " not found in the working project.";
    return "Activity " + std::to_string(activityId) + " manager updated to employee ID " + std::to_string(managerId) + ".";
}

std::string AgentTools::updateActivityPlannedStart(int activityId, const std::string& date) {
    auto stmt = db_.prepareStatement(
        "UPDATE Activities a "
        "JOIN ActivitySubcategories acs ON a.subcategoryid = acs.subcategoryid "
        "JOIN ActivityCategories ac ON acs.categoryid = ac.categoryid "
        "SET a.plannedstart = ? "
        "WHERE a.activityid = ? AND ac.projectid = ?"
    );
    stmt->setString(1, date);
    stmt->setInt(2, activityId);
    stmt->setInt(3, workingProjectID_);
    int rows = stmt->executeUpdate();
    if (rows == 0) return "Error: activity ID " + std::to_string(activityId) + " not found in the working project.";
    return "Activity " + std::to_string(activityId) + " planned start updated to " + date + ".";
}

std::string AgentTools::updateActivityPlannedFinish(int activityId, const std::string& date) {
    auto stmt = db_.prepareStatement(
        "UPDATE Activities a "
        "JOIN ActivitySubcategories acs ON a.subcategoryid = acs.subcategoryid "
        "JOIN ActivityCategories ac ON acs.categoryid = ac.categoryid "
        "SET a.plannedfinish = ? "
        "WHERE a.activityid = ? AND ac.projectid = ?"
    );
    stmt->setString(1, date);
    stmt->setInt(2, activityId);
    stmt->setInt(3, workingProjectID_);
    int rows = stmt->executeUpdate();
    if (rows == 0) return "Error: activity ID " + std::to_string(activityId) + " not found in the working project.";
    return "Activity " + std::to_string(activityId) + " planned finish updated to " + date + ".";
}

std::string AgentTools::updateActivityPlannedHours(int activityId, double hours) {
    auto stmt = db_.prepareStatement(
        "UPDATE Activities a "
        "JOIN ActivitySubcategories acs ON a.subcategoryid = acs.subcategoryid "
        "JOIN ActivityCategories ac ON acs.categoryid = ac.categoryid "
        "SET a.plannedhours = ? "
        "WHERE a.activityid = ? AND ac.projectid = ?"
    );
    stmt->setDouble(1, hours);
    stmt->setInt(2, activityId);
    stmt->setInt(3, workingProjectID_);
    int rows = stmt->executeUpdate();
    if (rows == 0) return "Error: activity ID " + std::to_string(activityId) + " not found in the working project.";
    return "Activity " + std::to_string(activityId) + " planned hours updated to " + std::to_string(hours) + ".";
}

std::string AgentTools::updateActivityForecastStart(int activityId, const std::string& date) {
    auto stmt = db_.prepareStatement(
        "UPDATE Activities a "
        "JOIN ActivitySubcategories acs ON a.subcategoryid = acs.subcategoryid "
        "JOIN ActivityCategories ac ON acs.categoryid = ac.categoryid "
        "SET a.forecaststart = ? "
        "WHERE a.activityid = ? AND ac.projectid = ?"
    );
    stmt->setString(1, date);
    stmt->setInt(2, activityId);
    stmt->setInt(3, workingProjectID_);
    int rows = stmt->executeUpdate();
    if (rows == 0) return "Error: activity ID " + std::to_string(activityId) + " not found in the working project.";
    return "Activity " + std::to_string(activityId) + " forecast start updated to " + date + ".";
}

std::string AgentTools::updateActivityForecastFinish(int activityId, const std::string& date) {
    auto stmt = db_.prepareStatement(
        "UPDATE Activities a "
        "JOIN ActivitySubcategories acs ON a.subcategoryid = acs.subcategoryid "
        "JOIN ActivityCategories ac ON acs.categoryid = ac.categoryid "
        "SET a.forecastfinish = ? "
        "WHERE a.activityid = ? AND ac.projectid = ?"
    );
    stmt->setString(1, date);
    stmt->setInt(2, activityId);
    stmt->setInt(3, workingProjectID_);
    int rows = stmt->executeUpdate();
    if (rows == 0) return "Error: activity ID " + std::to_string(activityId) + " not found in the working project.";
    return "Activity " + std::to_string(activityId) + " forecast finish updated to " + date + ".";
}

std::string AgentTools::updateActivityForecastHours(int activityId, double hours) {
    auto stmt = db_.prepareStatement(
        "UPDATE Activities a "
        "JOIN ActivitySubcategories acs ON a.subcategoryid = acs.subcategoryid "
        "JOIN ActivityCategories ac ON acs.categoryid = ac.categoryid "
        "SET a.forecasthours = ? "
        "WHERE a.activityid = ? AND ac.projectid = ?"
    );
    stmt->setDouble(1, hours);
    stmt->setInt(2, activityId);
    stmt->setInt(3, workingProjectID_);
    int rows = stmt->executeUpdate();
    if (rows == 0) return "Error: activity ID " + std::to_string(activityId) + " not found in the working project.";
    return "Activity " + std::to_string(activityId) + " forecast hours updated to " + std::to_string(hours) + ".";
}

std::string AgentTools::updateTaskDescription(int taskId, const std::string& description) {
    auto stmt = db_.prepareStatement(
        "UPDATE ActivityTasks t "
        "JOIN Activities a ON t.activityid = a.activityid "
        "JOIN ActivitySubcategories acs ON a.subcategoryid = acs.subcategoryid "
        "JOIN ActivityCategories ac ON acs.categoryid = ac.categoryid "
        "SET t.taskdescription = ? "
        "WHERE t.taskid = ? AND ac.projectid = ?"
    );
    stmt->setString(1, description);
    stmt->setInt(2, taskId);
    stmt->setInt(3, workingProjectID_);
    int rows = stmt->executeUpdate();
    if (rows == 0) return "Error: task ID " + std::to_string(taskId) + " not found in the working project.";
    return "Task " + std::to_string(taskId) + " description updated.";
}

std::string AgentTools::updateTaskDepartment(int taskId, int departmentId) {
    auto stmt = db_.prepareStatement(
        "UPDATE ActivityTasks t "
        "JOIN Activities a ON t.activityid = a.activityid "
        "JOIN ActivitySubcategories acs ON a.subcategoryid = acs.subcategoryid "
        "JOIN ActivityCategories ac ON acs.categoryid = ac.categoryid "
        "SET t.departmentid = ? "
        "WHERE t.taskid = ? AND ac.projectid = ?"
    );
    stmt->setInt(1, departmentId);
    stmt->setInt(2, taskId);
    stmt->setInt(3, workingProjectID_);
    int rows = stmt->executeUpdate();
    if (rows == 0) return "Error: task ID " + std::to_string(taskId) + " not found in the working project.";
    return "Task " + std::to_string(taskId) + " department updated to department ID " + std::to_string(departmentId) + ".";
}

std::string AgentTools::getWorkingProject() {
    if (workingProjectID_ == 0) return "No working project is currently set.";
    auto stmt = db_.prepareStatement(
        "SELECT projectid, projectno, projectname FROM Projects WHERE projectid = ?"
    );
    stmt->setInt(1, workingProjectID_);
    auto res = stmt->executeQuery();
    if (!res->next()) return "Error: working project ID " + std::to_string(workingProjectID_) + " not found.";
    std::ostringstream ss;
    ss << "## Current Working Project\n";
    ss << "| Project ID | Project Number | Project Name |\n";
    ss << "| --- | --- | --- |\n";
    ss << "| " << res->getInt("projectid") << " | " << res->getString("projectno") << " | " << res->getString("projectname") << " |\n";
    return ss.str();
}

std::string AgentTools::getTaskAssignments(int employeeId, int taskId) {
    auto stmt = db_.prepareStatement(
        "SELECT ata.assignmentid, ata.taskid, ata.userid, "
        "CONCAT(e.firstname, ' ', e.lastname) AS employeename, "
        "COALESCE(DATE_FORMAT(ata.assigneddate, '%Y-%m-%d'), '') AS assigneddate, "
        "COALESCE(DATE_FORMAT(ata.closedate, '%Y-%m-%d'), '') AS closedate "
        "FROM ActivityTaskAssignments ata "
        "JOIN Employees e ON ata.userid = e.employeeid "
        "WHERE ata.userid = ? AND ata.taskid = ?"
    );
    stmt->setInt(1, employeeId);
    stmt->setInt(2, taskId);
    auto res = stmt->executeQuery();
    std::ostringstream ss;
    ss << "## Task Assignments\n";
    ss << "| Assignment ID | Task ID | Employee ID | Employee Name | Assigned Date | Close Date |\n";
    ss << "| --- | --- | --- | --- | --- | --- |\n";
    while (res->next()) {
        ss << "| " << res->getInt("assignmentid")
           << " | " << res->getInt("taskid")
           << " | " << res->getInt("userid")
           << " | " << res->getString("employeename")
           << " | " << res->getString("assigneddate")
           << " | " << res->getString("closedate")
           << " |\n";
    }
    return ss.str();
}

std::string AgentTools::getEmployeeTaskTimeSummary(int employeeId, int taskId) {
    auto stmt = db_.prepareStatement(
        "SELECT e.employeeid, t.taskid, t.taskname, COALESCE(SUM(h.hours), 0) AS hrs "
        "FROM Employees e "
        "JOIN ActivityTaskAssignments ata ON ata.userid = e.employeeid "
        "JOIN Hours h ON h.assignmentid = ata.assignmentid "
        "JOIN ActivityTasks t ON ata.taskid = t.taskid "
        "WHERE e.employeeid = ? AND t.taskid = ? "
        "GROUP BY e.employeeid, t.taskid, t.taskname"
    );
    stmt->setInt(1, employeeId);
    stmt->setInt(2, taskId);
    auto res = stmt->executeQuery();
    std::ostringstream ss;
    ss << "## Employee Task Time Booking Summary\n";
    ss << "| Employee ID | Task ID | Task Name | Hours Booked |\n";
    ss << "| --- | --- | --- | --- |\n";
    while (res->next()) {
        ss << "| " << res->getInt("employeeid")
           << " | " << res->getInt("taskid")
           << " | " << res->getString("taskname")
           << " | " << res->getDouble("hrs")
           << " |\n";
    }
    return ss.str();
}

std::string AgentTools::getTeamMembers() {
    if (workingProjectID_ == 0) return "Error: no working project set.";
    auto stmt = db_.prepareStatement(
        "SELECT ptm.teammemberid, ptm.employeeid, "
        "CONCAT(e.firstname, ' ', e.lastname) AS employeename, "
        "d.departmentname, COALESCE(ptm.role, 0) AS role "
        "FROM ProjectTeamMembers ptm "
        "JOIN Employees e ON ptm.employeeid = e.employeeid "
        "JOIN Departments d ON e.departmentid = d.departmentid "
        "WHERE ptm.projectid = ? "
        "ORDER BY d.departmentname, e.lastname, e.firstname"
    );
    stmt->setInt(1, workingProjectID_);
    auto res = stmt->executeQuery();
    std::ostringstream ss;
    ss << "## Project Team Members\n";
    ss << "| Team Member ID | Employee ID | Employee Name | Department | Role |\n";
    ss << "| --- | --- | --- | --- | --- |\n";
    while (res->next()) {
        ss << "| " << res->getInt("teammemberid")
           << " | " << res->getInt("employeeid")
           << " | " << res->getString("employeename")
           << " | " << res->getString("departmentname")
           << " | " << res->getInt("role")
           << " |\n";
    }
    return ss.str();
}

std::string AgentTools::addTeamMember(int employeeId, int role) {
    if (workingProjectID_ == 0) return "Error: no working project set.";
    if (role < 0 || role > 2) return "Error: invalid role. Must be 0 (Member), 1 (Lead), or 2 (Manager).";
    std::string cols = "INSERT INTO ProjectTeamMembers (projectid, employeeid";
    std::string vals = ") VALUES (?, ?";
    bool hasRole = role != 0;
    if (hasRole) { cols += ", role"; vals += ", ?"; }
    auto stmt = db_.prepareStatement(cols + vals + ")");
    stmt->setInt(1, workingProjectID_);
    stmt->setInt(2, employeeId);
    if (hasRole) stmt->setInt(3, role);
    stmt->executeUpdate();
    auto res = db_.executeQuery("SELECT LAST_INSERT_ID() AS id");
    res->next();
    std::ostringstream ss;
    ss << "Team member added. Team Member ID: " << res->getInt("id") << ", Employee ID: " << employeeId;
    return ss.str();
}

std::string AgentTools::updateTeamMemberRole(int teamMemberId, int role) {
    if (workingProjectID_ == 0) return "Error: no working project set.";
    if (role < 0 || role > 2) return "Error: invalid role. Must be 0 (Member), 1 (Lead), or 2 (Manager).";
    auto stmt = db_.prepareStatement(
        "UPDATE ProjectTeamMembers SET role = ? WHERE teammemberid = ? AND projectid = ?"
    );
    stmt->setInt(1, role);
    stmt->setInt(2, teamMemberId);
    stmt->setInt(3, workingProjectID_);
    int rows = stmt->executeUpdate();
    if (rows == 0) return "Error: team member not found in working project.";
    std::ostringstream ss;
    ss << "Team member role updated. Team Member ID: " << teamMemberId << ", New Role: " << role;
    return ss.str();
}

std::string AgentTools::getDepartments() {
    auto res = db_.executeQuery(
        "SELECT departmentid, departmentname FROM Departments ORDER BY departmentname"
    );
    std::ostringstream ss;
    ss << "## Department List\n";
    ss << "| Department ID | Department Name |\n";
    ss << "| --- | --- |\n";
    while (res->next()) {
        ss << "| " << res->getInt("departmentid")
           << " | " << res->getString("departmentname")
           << " |\n";
    }
    return ss.str();
}

} // namespace planner
