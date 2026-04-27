# Osoem

A database system designed specifically for engineering project management.

## Overview

Engineering projects generate large volumes of interconnected data — documents issued, time booked, activity progress, quality checks, and physical commodities. Conventional approaches combine multiple independent systems (payroll, scheduling, document control) that fail to capture the relationships between these data sets, leading to duplication and inconsistencies.

Osoem provides a unified database that models these relationships explicitly. Project data is organized around four core concepts:

- **People** — the team members executing the project
- **Activities** — the hierarchical breakdown of project work (Work Breakdown Structure)
- **Time** — hours booked against tasks
- **Artefacts** — all deliverables produced by the project (drawings, calculations, reports, etc.)

## Repository Structure

```
osoem/
├── database/           # MySQL schema and setup scripts
│   └── schema/
│       └── schema.sql  # Complete database schema
├── admin/              # Command-line administration tool (C++)
```

## Database

The database is implemented in MySQL. The schema covers:

| Table | Description |
|---|---|
| `Departments` | Organisational units |
| `Employees` | Team members, with authentication and admin flag |
| `Projects` | Top-level project records |
| `ProjectTeamMembers` | Per-project team assignments and roles |
| `ActivityCategories` / `ActivitySubcategories` | Two-level work breakdown structure |
| `Activities` | Planned/forecast/actual dates and hours per activity |
| `ActivityNotes` | Freeform notes attached to activities |
| `ActivityTasks` | Recursive task hierarchy within an activity |
| `ActivityTaskAssignments` | Task-to-employee assignments |
| `Hours` | Time bookings against assignments |
| `ArtefactTypes` | User-defined deliverable types per project |
| `ArtefactDataFields` | Custom fields for each artefact type |
| `Artefacts` | Individual deliverable instances |
| `ArtefactData` | Field values for each artefact |
| `ArtefactToActivityLink` | Links an artefact to one or more activities (with ratio) |
| `Milestones` / `MilestoneSteps` | Progress milestones with a progress ratio per step |
| `MilestoneToArtefactLink` | Records completion of milestone steps against artefacts |

### Setup

```bash
mysql -u <user> -p < database/create_database.sql
mysql -u <user> -p osoem < database/schema/schema.sql
```

To reset to a clean state:

```bash
mysql -u <user> -p osoem < database/reset_database.sql
```

## Admin Tool

A C++ interactive command-line tool for navigating and editing the database. See [`admin/README.md`](admin/README.md) for full documentation.

### Quick Start

```bash
cd admin
mkdir build && cd build
cmake ..
cd ..
cmake --build build/ --target=osoem_admin
./build/osoem_admin
```

The shell prompt shows the current location in the data hierarchy:

```
[osoem] ~/Projects/Building Project/Activity Categories>
```

Key commands: `ls`, `cd`, `ad`, `rm`, `st`, `im`, `ex`


## Dependencies

| Component | Requirements |
|---|---|
| Database | MySQL / MariaDB |
| Admin tool | CMake 3.10+, C++17, `libmysqlcppconn-dev`, `libreadline-dev`, `libssl-dev` |
