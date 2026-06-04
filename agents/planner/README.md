# Osoem Planner Agent

An autonomous planning agent for the Osoem engineering project management system. It reads project and employee data from the Osoem MySQL database and uses the OpenAI API to generate activity plans and resource allocations based on historical data.

## Dependencies

| Dependency | Purpose | Install |
|------------|---------|---------|
| MySQL Connector/C++ | Database access | `sudo apt install libmysqlcppconn-dev` |
| libcurl | HTTP requests to OpenAI API | `sudo apt install libcurl4-openssl-dev` |
| libreadline | Interactive CLI input | `sudo apt install libreadline-dev` |
| nlohmann/json | JSON parsing (fetched by CMake) | — |
| CMake 3.14+ | Build system | `sudo apt install cmake` |

## Setup

**1. Run the setup script** — copies config templates and builds the project:

```bash
./setup.sh
```

**2. Edit `config/db_config.h`** with your MySQL credentials:

```cpp
const std::string DB_HOST = "localhost";
const std::string DB_USER = "your_username";
const std::string DB_PASS = "your_password";
const std::string DB_NAME = "osoem_database";
const int DB_PORT = 3306;
```

**3. Edit `config/api_config.h`** with your OpenAI API key and model:

```cpp
const std::string OPENAI_API_KEY = "sk-...";
const std::string OPENAI_MODEL   = "gpt-4o";
const std::string OPENAI_API_URL = "https://api.openai.com/v1/chat/completions";
```

## Build

```bash
cmake -B build
cmake --build build --target osoem_planner
```

## Run

```bash
./build/osoem_planner
```

On startup, the agent connects to the database and prompts you to select a working project (by ID or name, with tab-completion). All write operations apply exclusively to that project for the session.

### Interactive commands

| Input | Effect |
|-------|--------|
| Any question or instruction | Sent to the AI; tool calls are handled automatically |
| `clear` | Resets the conversation history (system prompt is retained) |
| `exit` / `quit` | Exits the agent |

## Capabilities

The agent uses OpenAI function-calling to interact with the database on your behalf.

**Read tools** (can query any project):
- `get_projects` — list all projects
- `get_working_project` — show the active project
- `get_employees` / `get_departments` / `get_team_members` — organisational data
- `get_activities_in_project` — activities with category, manager, planned and actual hours
- `get_tasks_in_activity` — task hierarchy and responsible departments
- `get_employee_time_summary` — total hours booked by an employee
- `get_employee_task_time_summary` / `get_task_assignments` — per-task time detail

**Write tools** (working project only):
- `create_activity_category` / `create_activity_subcategory` / `create_activity` / `create_task`
- `update_activity_*` — description, manager, planned/forecast start, finish, hours
- `update_task_*` — description, department
- `add_team_member` / `update_team_member_role`

## Source layout

```
src/
  main.cpp                  — entry point
  data/
    database.h/.cpp         — MySQL connection and queries
  planner/
    openai_client.h/.cpp    — libcurl HTTP wrapper for the OpenAI chat API
    agent_tools.h/.cpp      — tool definitions and database-backed implementations
    planner.h/.cpp          — conversation loop, project selection, system prompt
  utils/
    utils.h/.cpp            — string utilities
config/
  db_config.h.example       — database credentials template
  api_config.h.example      — API key/model template
```
