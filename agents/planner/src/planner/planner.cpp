#include "planner.h"
#include "../utils/utils.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <readline/readline.h>
#include <readline/history.h>

namespace {

std::vector<std::string> g_projectNames;

char* projectNameGenerator(const char* text, int state) {
    static size_t idx, len;
    if (state == 0) {
        idx = 0;
        len = strlen(text);
    }
    while (idx < g_projectNames.size()) {
        const auto& name = g_projectNames[idx++];
        if (strncasecmp(name.c_str(), text, len) == 0)
            return strdup(name.c_str());
    }
    return nullptr;
}

char** projectCompletion(const char* text, int /*start*/, int /*end*/) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, projectNameGenerator);
}

} // anonymous namespace

namespace planner {

Planner::Planner(OpenAIClient& client, data::Database& db)
    : client_(client), tools_(db) {
    messages_.push_back({"system", buildSystemPrompt()});
}

std::string Planner::buildSystemPrompt() {
    std::ostringstream ss;

    ss << "You are the Osoem Project Planner, an AI assistant for engineering project management.\n"
       << "Your role is to analyse historical project data and help plan activities, estimate effort,\n"
       << "and recommend resource allocation for the current working project.\n\n"

       << "## Working Project\n"
       << "A working project is set at startup. All create and update operations apply exclusively to\n"
       << "the working project. Use get_working_project to confirm which project is active before\n"
       << "making any writes.\n\n"

       << "## Read Capabilities\n"
       << "- get_projects — list all projects and their IDs.\n"
       << "- get_working_project — show the current working project.\n"
       << "- get_employees — list all employees with their departments.\n"
       << "- get_department — list all the departments in the organisation.\n"
       << "- get_activities_in_project — list activities in any project, including category,\n"
       << "  subcategory, manager, planned start/finish, planned hours, and actual hours booked.\n"
       << "- get_tasks_in_activity — list tasks in an activity, including hierarchy (parent task),\n"
       << "  description, and responsible department.\n"
       << "- get_employee_time_summary — total hours booked by an employee on a project.\n"
       << "- get_employee_task_time_summary — hours booked by an employee on a specific task.\n"
       << "- get_task_assignments — assignment records for an employee on a specific task,\n"
       << "  including assigned and close dates.\n\n"

       << "## Write Capabilities (working project only)\n"
       << "Structure creation:\n"
       << "- create_activity_category — add a new top-level category to the working project.\n"
       << "- create_activity_subcategory — add a subcategory under an existing category.\n"
       << "- create_activity — create an activity under a subcategory, with optional manager,\n"
       << "  description, planned start/finish, and planned hours.\n"
       << "- create_task — create a task inside an activity, optionally nested under a parent task\n"
       << "  and assigned to a department.\n\n"
       << "Activity updates:\n"
       << "- update_activity_description, update_activity_manager\n"
       << "- update_activity_planned_start, update_activity_planned_finish, update_activity_planned_hours\n"
       << "- update_activity_forecast_start, update_activity_forecast_finish, update_activity_forecast_hours\n\n"
       << "Task updates:\n"
       << "- update_task_description, update_task_department\n\n"
       << "Team updates:\n"
       << "- add_team_member - add a new team member to the project.\n\n"

       << "## Guidelines\n"
       << "- Before creating anything, check whether a matching category or subcategory already exists.\n"
       << "- Always use the read capabilities to look for past projects that may be similar to the current working project.\n"
       << "- When estimating hours, reference comparable activities from historical project data.\n"
       << "- When suggesting team members, consider their department and past hours on similar tasks.\n"
       << "- When building task breakdowns, mirror patterns seen in similar historical activities.\n"
       << "- Be concise. Only provide the required data when requested for data. Do not give any recommendations or additional notes in addition to the required details.\n"
       << "- Before updating any data ask for confirmation.\n\n"
        
       << "## Data Validation Rules\n"
       << "When writing data to the database using the write capabilities always follow these validation rules:\n"
       << "- Name fields (category_name, subcategory_name, activity_name, task_name): Cannot contain reserved characters `,` `:` `/` `~`; Cannot start with a number or a `.`; Must not exceed 255 characters;\n"
       << "- Description fields: Cannot contain reserved characters `,` `:` `/` `~`; Cannot start with a number or a `.`; have a max 1000 characters;\n";

    return ss.str();
}

void Planner::selectWorkingProject() {
    auto projects = tools_.getProjectList();

    g_projectNames.clear();
    for (const auto& [id, name] : projects)
        g_projectNames.push_back(name);

    rl_attempted_completion_function = projectCompletion;

    while (true) {
        char* raw = readline("Select working project (ID or name): ");
        if (!raw) {
            std::cout << "\nNo project selected. Exiting.\n";
            exit(0);
        }
        std::string input(raw);
        free(raw);

        if (input.empty()) continue;

        bool matched = false;
        // Try numeric ID first
        try {
            size_t pos;
            int id = std::stoi(input, &pos);
            if (pos == input.size()) {
                for (const auto& [pid, pname] : projects) {
                    if (pid == id) {
                        tools_.setWorkingProject(id);
                        std::cout << "Working project set to: " << pname << "\n\n";
                        matched = true;
                        break;
                    }
                }
                if (!matched)
                    std::cout << "No project with ID " << id << " found.\n";
            }
        } catch (...) {}

        // Try name match (case-insensitive)
        if (!matched) {
            std::string lower = utils::trim(input);
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            for (const auto& [pid, pname] : projects) {
                std::string pnameLower = pname;
                std::transform(pnameLower.begin(), pnameLower.end(), pnameLower.begin(), ::tolower);
                if (pnameLower == lower) {
                    tools_.setWorkingProject(pid);
                    std::cout << "Working project set to: " << pname << "\n\n";
                    matched = true;
                    break;
                }
            }
            if (!matched)
                std::cout << "No project matching \"" << input << "\" found.\n";
        }

        if (matched) break;
    }

    rl_attempted_completion_function = nullptr;
    g_projectNames.clear();
}

void Planner::run() {
    std::cout << "Osoem Planner Agent\n"
              << "===================\n"
              << "Ask planning questions about projects, resources, or schedules.\n"
              << "Commands: 'clear' resets the conversation, 'exit' or 'quit' to stop.\n\n";

    selectWorkingProject();

    while (true) {
        std::string input = readInput("> ");
        if (input.empty()) continue;

        if (input == "exit" || input == "quit") {
            std::cout << "Goodbye.\n";
            break;
        }

        if (input == "clear") {
            // Keep only the system message
            messages_.erase(messages_.begin() + 1, messages_.end());
            std::cout << "Conversation cleared.\n\n";
            continue;
        }
        const size_t rollbackSize = messages_.size();
        messages_.push_back({"user", input});
        std::cout << "\n";
        try {
            while (true) {
                Response response = client_.chat(messages_, tools_.getTools());
                if (response.type == TOOL_CALL) {
                    messages_.push_back({"assistant", response.message, response.toolCalls});
                    for (const auto& tool : response.toolCalls) {
                        messages_.push_back({"tool", tools_.callTool(tool), {}, tool.id});
                    }
                } else {
                    messages_.push_back({"assistant", response.message});
                    std::cout << response.message << "\n\n";
                    break;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n\n";
            messages_.resize(rollbackSize);
        }
    }
}

std::string Planner::readInput(const std::string& prompt) {
    char* line = readline(prompt.c_str());
    if (!line) return "exit";
    std::string input(line);
    if (!input.empty()) add_history(line);
    free(line);
    return input;
}

} // namespace planner
