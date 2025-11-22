# OSOEM Admin Tool

A command-line administrative tool for managing the OSOEM engineering project management database.

## Features

The osoem_admin tool provides the following functionality:

1. **Import and manage activities** from CSV files
2. **Navigate, add, and remove activities** with hierarchical task management
3. **Add tasks to activities** with parent-child relationships
4. **Create artefact types** and import artefacts in bulk
5. **Add and remove employees**
6. **Add and remove milestones** with progress tracking steps
7. **Link artefacts to activities** with ratio allocation
8. **Link milestone steps to artefacts** for progress measurement

## Prerequisites

### System Requirements
- Linux or WSL (Windows Subsystem for Linux)
- CMake 3.10 or higher
- C++17 compatible compiler (g++ or clang++)
- MySQL/MariaDB server
- MySQL Connector/C++ library

### Installing Dependencies

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install cmake g++ libmysqlcppconn-dev
```

#### Fedora/RHEL
```bash
sudo dnf install cmake gcc-c++ mysql-connector-c++-devel
```

## Building the Project

1. Navigate to the admin directory:
```bash
cd admin
```

2. Create a build directory:
```bash
mkdir build
cd build
```

3. Run CMake:
```bash
cmake ..
```

4. Build the project:
```bash
make
```

5. The executable `osoem_admin` will be created in the build directory.

### Optional: Install System-Wide
```bash
sudo make install
```

## Configuration

The tool connects to the database using the following default settings (configured in `config/database.h`):

- **Host:** localhost
- **Port:** 3306
- **User:** roshn
- **Password:** P@ssword
- **Database:** trial

To change these settings, edit `config/database.h` and rebuild the project.

## Usage

### General Command Format
```bash
./osoem_admin <command> [options]
```

### Activity Management

**Import activities from CSV:**
```bash
./osoem_admin import-activities <csv_file> <project_id>
```
Example:
```bash
./osoem_admin import-activities ../data/sample/Residential_building_project/activities.csv 1
```

**List all activities:**
```bash
./osoem_admin list-activities [project_id]
```

**Add a single activity:**
```bash
./osoem_admin add-activity <name> <subcategory_id> <manager_id> <start_date> <end_date> <hours>
```
Example:
```bash
./osoem_admin add-activity "Structural Analysis" 5 3 2026-03-01 2026-05-15 320.0
```

**Remove an activity:**
```bash
./osoem_admin remove-activity <activity_id>
```

### Task Management

**Add a task to an activity:**
```bash
./osoem_admin add-task <activity_id> <task_name> [parent_task_id]
```
Examples:
```bash
# Add a top-level task
./osoem_admin add-task 5 "Review architectural drawings"

# Add a subtask
./osoem_admin add-task 5 "Check dimensions" 12
```

**List tasks for an activity:**
```bash
./osoem_admin list-tasks <activity_id>
```

### Artefact Management

**Create a new artefact type:**
```bash
./osoem_admin create-artefact-type <project_id> <name> <description>
```
Example:
```bash
./osoem_admin create-artefact-type 1 "Engineering Drawings" "Technical drawings for construction"
```

**Import artefacts from CSV:**
```bash
./osoem_admin import-artefacts <csv_file> <artefact_type_id>
```
Example:
```bash
./osoem_admin import-artefacts ../data/sample/Residential_building_project/drawings.csv 1
```

**List artefact types:**
```bash
./osoem_admin list-artefact-types [project_id]
```

**List artefacts of a specific type:**
```bash
./osoem_admin list-artefacts <artefact_type_id>
```

### Employee Management

**Add a new employee:**
```bash
./osoem_admin add-employee <firstname> <lastname> <email> <password>
```
Example:
```bash
./osoem_admin add-employee John Doe john.doe@example.com SecurePass123
```

**Remove an employee:**
```bash
./osoem_admin remove-employee <employee_id>
```

**List all employees:**
```bash
./osoem_admin list-employees
```

### Milestone Management

**Add a milestone:**
```bash
./osoem_admin add-milestone <project_id> <name>
```
Example:
```bash
./osoem_admin add-milestone 1 "Drawing Review Process"
```

**Add a milestone step:**
```bash
./osoem_admin add-milestone-step <milestone_id> <step_name> <progress_ratio>
```
Example:
```bash
./osoem_admin add-milestone-step 1 "Discipline Check" 0.50
./osoem_admin add-milestone-step 1 "Interdisciplinary Review" 0.75
./osoem_admin add-milestone-step 1 "Issue for Construction" 1.00
```

**Remove a milestone:**
```bash
./osoem_admin remove-milestone <milestone_id>
```

**List milestones:**
```bash
./osoem_admin list-milestones [project_id]
```

**List milestone steps:**
```bash
./osoem_admin list-milestone-steps <milestone_id>
```

### Linking Functionality

**Link an artefact to an activity:**
```bash
./osoem_admin link-artefact <artefact_id> <activity_id> <ratio>
```
Example:
```bash
# Link artefact 10 to activity 5 with 100% allocation
./osoem_admin link-artefact 10 5 1.00

# Split artefact 11 between two activities (50% each)
./osoem_admin link-artefact 11 5 0.50
./osoem_admin link-artefact 11 6 0.50
```

**Link a milestone step to an artefact:**
```bash
./osoem_admin link-milestone <milestone_step_id> <artefact_link_id>
```

**List artefact-activity links:**
```bash
./osoem_admin list-artefact-links [activity_id]
```

**List milestone links:**
```bash
./osoem_admin list-milestone-links [milestone_id]
```

### Help

**Display help:**
```bash
./osoem_admin help
./osoem_admin --help
./osoem_admin -h
```

## CSV File Formats

### Activities CSV (activities.csv)
```csv
Activity,Start_Date,End_Date,Manager
Project Initiation,2026-01-05,2026-01-19,Anne Mills
Site Survey,2026-01-20,2026-02-10,John Smith
```

**Fields:**
- `Activity`: Activity name (required)
- `Start_Date`: Planned start date in YYYY-MM-DD format (required)
- `End_Date`: Planned end date in YYYY-MM-DD format (required)
- `Manager`: Full name of the activity manager (required, will be created if doesn't exist)

### Artefacts CSV (drawings.csv)
```csv
Number,Owner,Title
A-001,Sarah Chen,Site Plan and Location Map
A-100,Sarah Chen,Ground Floor Plan
```

**Fields:**
- `Number`: Artefact identifier (mapped to custom fields if defined)
- `Owner`: Full name of the artefact owner (optional, will be created if doesn't exist)
- `Title`: Artefact title (used as artefacttitle in database)

**Note:** The CSV columns should match the data fields defined for the artefact type.

## Example Workflow

Here's a typical workflow for setting up a new project:

```bash
# 1. List existing employees
./osoem_admin list-employees

# 2. Add project manager if needed
./osoem_admin add-employee Anne Mills anne.mills@example.com password123

# 3. Import activities from CSV (assuming project ID is 1)
./osoem_admin import-activities ../data/sample/Residential_building_project/activities.csv 1

# 4. List imported activities
./osoem_admin list-activities 1

# 5. Add tasks to an activity (e.g., activity ID 5)
./osoem_admin add-task 5 "Review architectural drawings"
./osoem_admin add-task 5 "3D model preparation"
./osoem_admin add-task 5 "Prepare preliminary 3D model" 2

# 6. Create an artefact type
./osoem_admin create-artefact-type 1 "Drawings" "Engineering drawings"

# 7. Import artefacts
./osoem_admin import-artefacts ../data/sample/Residential_building_project/drawings.csv 1

# 8. Create a milestone
./osoem_admin add-milestone 1 "Drawing Review"
./osoem_admin add-milestone-step 1 "Discipline Check" 0.50
./osoem_admin add-milestone-step 1 "Issue for Construction" 1.00

# 9. Link artefacts to activities
./osoem_admin link-artefact 1 5 1.00

# 10. Link milestone steps to artefacts
./osoem_admin link-milestone 1 1
```

## Troubleshooting

### MySQL Connection Errors
- Verify MySQL service is running: `sudo systemctl status mysql`
- Check database credentials in `config/database.h`
- Ensure the database 'trial' exists
- Verify user 'roshn' has appropriate permissions

### Build Errors
- Ensure MySQL Connector/C++ is installed: `dpkg -l | grep libmysqlcppconn`
- Check CMake can find the library: Review CMake output messages
- Try specifying library path manually in CMakeLists.txt

### Import Errors
- Verify CSV file format matches expected structure
- Check for encoding issues (should be UTF-8)
- Ensure project_id exists in Projects table before importing

## Project Structure

```
admin/
├── CMakeLists.txt          # Build configuration
├── config/
│   └── database.h          # Database connection settings
├── src/
│   ├── main.cpp           # Entry point and command routing
│   ├── database.cpp/.h    # Database connection management
│   ├── csv_parser.cpp/.h  # CSV parsing utilities
│   ├── utils.cpp/.h       # Helper functions
│   └── commands/          # Command implementations
│       ├── activities.cpp/.h
│       ├── tasks.cpp/.h
│       ├── artefacts.cpp/.h
│       ├── employees.cpp/.h
│       ├── milestones.cpp/.h
│       └── links.cpp/.h
└── README.md              # This file
```

## License

This project is part of the OSOEM engineering project management system.
