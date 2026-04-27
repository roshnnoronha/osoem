# OSOEM Admin Tool

A command-line administrative tool for managing the OSOEM engineering project management database. The tool provides an interactive shell with a hierarchical folder-like interface for navigating and manipulating project data.

## Prerequisites

### System Requirements
- Linux or WSL (Windows Subsystem for Linux)
- CMake 3.10 or higher
- C++17 compatible compiler (g++ or clang++)
- MySQL/MariaDB server

### Installing Dependencies

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install cmake g++ libmysqlcppconn-dev libreadline-dev libssl-dev
```

#### Fedora/RHEL
```bash
sudo dnf install cmake gcc-c++ mysql-connector-c++-devel readline-devel openssl-devel
```

## Building the Project

```bash
cd admin
mkdir build
cd build
cmake ..
cd ..
cmake --build build/ --target=osoem_admin
```

The executable `osoem_admin` will be created in the `build/` directory.

### Optional: Install System-Wide
```bash
sudo make install
```

## Configuration

Database connection settings are configured in `config/db_config.h`. Edit this file and rebuild to change the connection parameters. Provide the <username>, <password> to the database.

## Usage

### Interactive Mode
```bash
./osoem_admin                          # Start as admin (default)
./osoem_admin -u user@example.com      # Start with user authentication
```

When using the `-u` flag, the tool prompts for a password and authenticates against the database. Authenticated users are assigned a role (Admin or User). Non-admin users have read-only access and cannot use data modification commands (`add`, `remove`, `set`, `import`).

### Batch Mode
```bash
./osoem_admin script.src               # Execute commands from a file
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
 ├── Organization
 │   └── Departments
 │       └── [Department]
 │           └── Employees
 └── Projects
     └── [Project]
         ├── Activity Categories
         │   └── [Category]
         │       └── [Subcategory]
         │           └── [Activity]
         │               ├── Tasks
         │               │   └── [Task]
         │               │       ├── Tasks (sub-tasks, recursive)
         │               │       └── Assignments
         │               │           └── [Assignment]
         │               │               └── Hours
         │               └── Notes
         ├── Artefact Types
         │   └── [Artefact Type]
         │       ├── Artefacts
         │       │   └── [Artefact]
         │       │       ├── Data
         │       │       └── Associations
         │       │           └── [Artefact-Activity Link]
         │       │               └── Milestone-Artefact Links
         │       └── Fields
         ├── Milestones
         │   └── [Milestone]
         │       └── Milestone Steps
         └── Team
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
| Team | `employeename, role` (role: 1=Member, 2=Manager, 3=Admin) |
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

The CSV file must have a header row with column names matching the expected fields for the current location. The `-r` (recursive) flag is used in batch scripts for bulk population where items may already exist across multiple import passes.

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

- Names cannot start with a number
- The `:` character is reserved for key-value syntax
- Foreign key references are validated against the database
- Artefact data fields are validated by type (text length, numeric range)

### Authentication and Authorization

When started with `-u <email>`, the tool authenticates against the database using SHA-256 hashed passwords with salt. Non-admin users are restricted to read-only commands (`list`, `select`, `export`). Without the `-u` flag, the tool runs with admin privileges.

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
./osoem_admin setup_project.src
```

## Examples

The `examples/` directory contains ready-to-run scripts and CSV data files for several engineering projects.

### Organization Examples (`examples/Organization_Examples/`)

Sets up departments and employees for a typical engineering organization.

```bash
./osoem_admin examples/Organization_Examples/departments.src
```

### Residential Building Project (`examples/Residential_Building_Project/`)

A complete residential building project demonstrating the activity and task hierarchy.

```bash
./osoem_admin examples/Residential_Building_Project/project.src
```

### Industrial Building Project (`examples/Industrial_Building_Project/`)

An industrial building project example.

```bash
./osoem_admin examples/Industrial_Building_Project/project.src
```

### Data Center Design (`examples/Data_Center_Design/`)

A data center design project with artefact types, fields, milestones, and CSV data files for activities and artefacts.

```bash
./osoem_admin examples/Data_Center_Design/Data_Center_Design.src
```

### Water Treatment Plant (`examples/Water_Treatment_Plant/`)

A large-scale example with multiple engineering disciplines, artefact types (drawings, calculations, datasheets), milestones, and a comprehensive activity breakdown. Data is organised into per-discipline CSV files.

```bash
./osoem_admin examples/Water_Treatment_Plant/Water_Treatment_Plant.src
```

### CSV Examples (`examples/CSV_Examples/`)

Standalone CSV files for each entity type, useful as templates when importing data.

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
