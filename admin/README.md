# OSOEM Admin Tool

A command-line administrative tool for managing the OSOEM engineering project management database. The tool provides an interactive shell with a hierarchical folder-like interface for navigating and manipulating project data.

## Prerequisites

- Linux or WSL (Windows Subsystem for Linux)
- CMake 3.10 or higher
- C++17 compatible compiler (g++)
- MySQL/MariaDB server
- Required libraries: `libmysqlcppconn-dev`, `libreadline-dev`, `libssl-dev`

## Setup

### 1. Create the database

Run the database setup script from the `database/` directory (see: ../database/README.md for more information). This drops and recreates the `osoem_database` MySQL database, applies the schema, and inserts a temporary admin account for first-time login.

### 2. Build the admin tool

Run the setup script from the `admin/` directory:

```bash
./setup.sh
```

This will:
1. Prompt for database connection details (host, port, user, password) and write `config/db_config.h`
2. Configure the CMake build (creates the `build/` directory if it does not exist)
3. Compile the `osoem_admin` executable to `build/osoem_admin`

Options:
```
./setup.sh -d    # Install build dependencies via apt (requires sudo), then build
./setup.sh -c    # Reconfigure db_config.h (overwrite existing), then build
./setup.sh -cd   # Both: install deps and reconfigure
```

The `-d` flag installs the following packages via `apt`:
```bash
sudo apt install cmake g++ libmysqlcppconn-dev libreadline-dev libssl-dev
```

For Fedora/RHEL, install dependencies manually before running `./setup.sh`:
```bash
sudo dnf install cmake gcc-c++ mysql-connector-c++-devel readline-devel openssl-devel
```

### 3. First-time login

The database setup creates a temporary admin account (`email: admin`, no password). Use it to create your real admin employee, then delete it.

**Log in as the temporary admin:**
```bash
./build/osoem_admin -u admin
# Press Enter when prompted for password (no password set)
```

**Create a department and your admin employee:**
```
cd Organization/Departments
ad "<department_name>"
cd "<department_name>"
ad
# Fill in: firstname, lastname, email, password
```

**Log out and log back in as your real admin account:**
```bash
./build/osoem_admin -u your@email.com
```

> **Security warning:** Delete the temporary admin account after creating your real one. It has no password and full admin privileges.
> ```
cd Organization/Departments/IT
> rm admin
> ```

## Usage

### Interactive Mode
```bash
./build/osoem_admin -u user@example.com    
```

When using the `-u` flag, the tool prompts for a password and authenticates against the database. Authenticated users are assigned a role (Admin or User). Non-admin users have read-only access and cannot use data modification commands (`add`, `remove`, `set`, `import`).

### Batch Mode
```bash
./build/osoem_admin script.src         # Execute commands from a file
```

In batch mode, commands are read from a text file and executed sequentially. Lines starting with `#` are treated as comments. The tool displays the file name and line number during execution. Batch mode runs with admin privileges.

## Navigating the Hierarchy

The tool organizes data in a folder-like hierarchy. The prompt displays the current location:

```
[osoem] ~/Projects/Building Project/Activity Categories>
```

The root is represented by `~`. The full hierarchy is:

```
~ (root)
 ├── Organization **[Folder]**
 │   └── Departments
 │       └── Employees
 └── Projects **[Folder]**
     └── Projects
         ├── ActivityCategories **[Folder]**
         │   └── Categories
         │       └── Sub-categories
         │           └── Activities
         │               ├── Tasks **[Folder]**
         │               │   └── Tasks
         │               │       ├── Tasks **[Folder]**
         │               │       │   └── Tasks
         │               │       │       ├── Tasks **[Folder]**
         │               │       │       │   └── Tasks
         │               │       │       │       : 
         │               │       │       └── Assignments **[Folder]**
         │               │       │           └── Assignments
         │               │       │               └── Hours
         │               │       └── Assignments **[Folder]**
         │               │           └── Assignments
         │               │               └── Hours
         │               └── Notes **[Folder]**
         │                   └── Notes
         ├── ArtefactTypes **[Folder]**
         │   └── ArtefactTypes
         │       ├── Artefacts **[Folder]**
         │       │   └── Artefacts
         │       │       ├── Data **[Folder]**
         │       │       │   └── ArtefactData
         │       │       └── Associations **[Folder]**
         │       │           └── ArtefactToActivityLinks
         │       │               └── MilestoneToArtefactLinks
         │       └── Fields **[Folder]**
         │           └── ArtefactFields
         ├── Milestones **[Folder]**
         │   └── Milestones
         │       └── MilestoneSteps
         └── Team **[Folder]**
             └── ProjectTeamMembers
```

## Commands

### list (ls)

Lists items at the current location.

```
ls          # Show ID and name
ls -a       # Show all fields in tabular format
```

### select (cd, sl)

Navigates to an item or folder. Supports selection by ID or by name (case-insensitive).

```
cd Projects                     # Navigate to a folder
cd "Building Project"           # Navigate to an item by name
cd 1                            # Navigate to an item by ID
cd ..                           # Go up one level
cd ~                            # Return to root
```

For employees, selection works with full name, first name, or last name.

### add (ad)

Adds a new item at the current location. Requires admin privileges.

```
ad                                          # Interactive mode (prompts for fields)
ad "Project Alpha,PA-001"                   # CSV values (positional)
ad "taskname:Review,assigneeid:3"           # Key-value pairs
```

The expected fields depend on the current location in the hierarchy:

| Location | Fields |
|---|---|
| Departments | `departmentname` |
| Employees | `firstname, lastname, email, password` |
| Projects | `projectname, projectno` |
| Team | `employeename, role` (role: 0=Member, 1=Lead, 2=Manager) |
| Activity Categories | `categoryname` |
| Subcategories | `subcategoryname` |
| Activities | `activityname, managerid, plannedstart, plannedfinish, plannedhours` |
| Tasks / Sub-tasks | `taskname` |
| Assignments | `employeename, assigneddate, closedate` |
| Hours | `hours, bookeddate` |
| Notes | `note, notedate` |
| Artefact Types | `artefactname, artefactdescription` |
| Artefacts | `title, artefactownername` |
| Fields | `fieldtitle, valuetype, maximumlength, maximumvalue, minimumvalue` |
| Artefact Data | `fieldname, value` |
| Associations (Artefact-Activity Links) | `activityname, ratio` |
| Milestone-Artefact Links | `milestonestepname, completedbyname, completiondate` |
| Milestones | `milestonename` |
| Milestone Steps | `milestonestepname, progressratio` |

Name-based lookups are supported for foreign key fields (e.g., using an employee name instead of an ID for `managerid`).

### remove (rm)

Removes an item by ID or name. Requires admin privileges.

```
rm 5                    # Remove by ID
rm "Project Alpha"      # Remove by name
```

### set (st)

Updates one or more fields on an item at the current location. Requires admin privileges. The first argument identifies the item (by ID or name); the second argument is a key-value CSV string using the same field names as `add`.

```
st 1 "projectname:New Project Name"
st "Project Alpha" "projectname:Refinery Project,projectno:RP-002"
st 3 "email:newemail@example.com"
```

The `set` command supports all entity types in the hierarchy, using the same field names as the `add` command.

### import (im)

Imports data from a CSV file into the current location. Requires admin privileges.

```
im employees.csv            # Normal import — error on duplicate entries
im -i employees.csv         # Ignore duplicates (skip silently)
im -r activities.csv        # Recursive import — resolves parent references and ignores duplicates
```

The CSV file must have a header row with column names matching the expected fields for the current location. The `-r` (recursive) flag is used in batch scripts for bulk population from a single large table.

### export (ex)

Exports data at the current location to a CSV file.

```
ex output.csv
```

### exit / quit

Exits the interactive shell. Ctrl+D also exits.

## Features

### Tab Completion

The interactive shell provides context-aware tab completion for commands, folder names, and item names. Names containing spaces are automatically quoted.

### Name-Based Lookups

Most commands accept names in addition to numeric IDs. Name matching is case-insensitive. If multiple matches are found, the first match is used with a warning.

### Input Validation

**Name fields** (departments, projects, categories, subcategories, activities, tasks, milestones, milestone steps, artefact types, artefacts, fields, and employee first/last names):
- Cannot be empty
- Cannot contain reserved characters: `,  :  /  ~`
- Cannot start with a number or a `.`
- Must not exceed 255 characters

**Special string fields:**
- `projectno`: cannot be empty or contain `,` or `:`; max 255 characters (may start with a number)
- Employee `email`: cannot be empty, must contain `@`, max 255 characters
- Employee `password`: cannot be empty, max 255 characters
- Activity/artefact type `description`: cannot be empty; cannot contain `,` or `:`; max 1000 characters
- Notes: cannot be empty; max 2000 characters
- Task `description`: max 2000 characters
- Artefact data `value`: max 2000 characters

**Date fields** (`plannedstart`, `plannedfinish`, `assigneddate`, `closedate`, `bookeddate`, etc.):
- Must be in `YYYY-MM-DD` format
- Start date must not be after end date
- Assignment `closedate` must not be before `assigneddate`
- Assignment `assigneddate` cannot be after the activity's actual finish date

**Numeric fields:**
- `hours`: must be a positive integer
- `ratio` (artefact-activity link): must be between 0.0 and 1.0
- `progressratio` (milestone steps): must be between 0 and 1
- `valuetype` (artefact data field): must be `0` (text) or `1` (numeric)
- Team member `role`: must be `0` (Member), `1` (Lead), or `2` (Manager)
- `maximumlength` (text fields): must be positive and not exceed 2000
- `minimumvalue` must not exceed `maximumvalue` (numeric fields)

**Foreign key and relationship constraints:**
- Activity manager must be a member of the project team
- Artefact owner must be a member of the project team
- Assignment employee must be a member of the project team
- A team member cannot be removed or have their employee changed if they are the activity manager or artefact owner for any item in the project

**Import-specific:**
- Ambiguous `parenttaskname` references (matching multiple tasks) must be disambiguated by also supplying `activityname`, `subcategoryname`, or `categoryname`

### Authentication and Authorization

The tool authenticates against the database using SHA-256 hashed passwords with salt. Non-admin users are restricted to read-only commands (`list`, `select`, `export`).

### Batch Scripting

Command scripts can be executed by passing a filename as an argument. Example script:

```bash
# setup_project.src
cd Projects
ad "Refinery Project,RP-001"
cd "Refinery Project"
cd "Activity Categories"
ad "Engineering"
cd Engineering
ad "Structural"
cd Structural
ad "Foundation Design,1,2026-01-01,2026-06-01,500"
```

Run with:
```bash
./build/osoem_admin setup_project.src
```

The `examples/` directory contains ready-to-run scripts and CSV data files for several sample projects.

## Project Structure

```
admin/
├── CMakeLists.txt              # Build configuration
├── config/
│   └── db_config.h             # Database connection settings
├── src/
│   ├── main.cpp                # Entry point, interactive loop, argument parsing
│   ├── commands/               # Command implementations
│   │   ├── execute.cpp/.h      # Command dispatcher and batch execution
│   │   ├── list.cpp/.h         # list command
│   │   ├── select.cpp/.h       # select command
│   │   ├── add.cpp/.h          # add command
│   │   ├── remove.cpp/.h       # remove command
│   │   ├── set.cpp/.h          # set command
│   │   ├── importcsv.cpp/.h    # import command
│   │   └── exportcsv.cpp/.h    # export command
│   ├── data/                   # Database access layer
│   │   ├── database.cpp/.h     # Database connection management
│   │   ├── path.cpp/.h         # Hierarchy path state
│   │   ├── datarecord.h        # Base class for data records
│   │   └── *data.cpp/.h        # Data access for each entity type
│   ├── utils/                  # Utilities
│   │   ├── auth.cpp/.h         # Authentication (SHA-256 + salt)
│   │   ├── csv_parser.cpp/.h   # CSV parsing
│   │   ├── lookup.cpp/.h       # Name-to-ID lookups
│   │   ├── utils.cpp/.h        # General helpers
│   │   ├── completion_provider.cpp/.h  # Tab completion
│   │   └── readline_wrapper.cpp/.h     # Readline integration
│   └── exceptions/             # Custom exception types
├── examples/                   # Example scripts and CSV files
└── README.md                   # This file
```

## Troubleshooting

### MySQL Connection Errors
- Verify MySQL service is running: `sudo systemctl status mysql`
- Check database credentials in `config/db_config.h`
- Ensure the configured database exists
- Verify the database user has appropriate permissions

### Build Errors
- Ensure all dependencies are installed: `libmysqlcppconn-dev`, `libreadline-dev`, `libssl-dev`
- Check CMake output for missing library messages
- Verify C++17 support in your compiler

### Import Errors
- Verify CSV file headers match expected field names
- Check for encoding issues (should be UTF-8)
- Ensure you are at the correct location in the hierarchy before importing
