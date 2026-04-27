# Osoem Admin Tool — Test Plan

## Overview

This document is the canonical test reference for the osoem admin tool (`admin/`). It covers manual verification of all commands, navigation paths, entity types, validation rules, and error conditions. Each test case specifies a precondition, the input/steps to perform, and the expected result.

---

## Table of Contents

1. [Environment Setup](#1-environment-setup)
2. [Navigation — `select` and `list`](#2-navigation--select-and-list)
3. [List Command](#3-list-command)
4. [Add Command](#4-add-command)
5. [Remove Command](#5-remove-command)
6. [Set Command](#6-set-command)
7. [Import Command](#7-import-command)
8. [Export Command](#8-export-command)
9. [Authentication and Authorization](#9-authentication-and-authorization)
10. [Batch Execution](#10-batch-execution)
11. [Input Parsing and Edge Cases](#11-input-parsing-and-edge-cases)
12. [Data Integrity](#12-data-integrity)
13. [Help Command](#13-help-command)

---

## 1. Environment Setup

### 1.1 Prerequisites

| Requirement | Detail |
|---|---|
| MySQL server | Running on `localhost:3306` |
| Database name | `trial` (see `config/db_config.h`) |
| DB user | Configured in `config/db_config.h` |
| Compiled binary | Build with `cmake` + `make` from `admin/` |
| OpenSSL | Required for password hashing (`SHA256`) |
| GNU Readline | Required for interactive tab completion |

### 1.2 Resetting the Database to a Known State

Run the following from the `database/` directory in the MySQL CLI to drop, recreate, and repopulate the schema with sample data:

```sql
source reset_database.sql
```

`reset_database.sql` drops the `trial` database, recreates it, applies the schema from `schema/schema.sql`, and seeds employees and a residential building project.

To start with a completely empty schema (no seed data), run only:

```sql
source create_database.sql
```

### 1.3 Running the Admin Tool

**Interactive mode (no authentication):**
```bash
./osoem
```
> Note: `UserContext` currently defaults `isAdmin = true` (see `src/utils/auth.h` line 19). This means the unauthenticated mode currently grants admin access. Tests marked **[AU]** verify the intended authenticated behavior.

**Interactive mode with authentication:**
```bash
./osoem -u admin@email.com
```

**Batch mode (script file):**
```bash
./osoem examples/Organization_Examples/departments.src
./osoem examples/Residential_Building_Project/project.src
```

Batch mode always runs with `isAdmin = 1` regardless of the `-u` flag.

### 1.4 Test Data

Use the following files to bootstrap a complete test dataset:

| File | Purpose |
|---|---|
| `examples/Organization_Examples/departments.src` | Creates departments and employees |
| `examples/Residential_Building_Project/project.src` | Creates a full project with all entity types |
| `examples/CSV_Examples/employeedata.csv` | Bulk employee import |
| `examples/CSV_Examples/projectdata.csv` | Bulk project import |
| `examples/CSV_Examples/categorydata.csv` | Bulk category import |
| `examples/CSV_Examples/activitydata.csv` | Bulk activity import |
| `examples/CSV_Examples/artefactdata.csv` | Bulk artefact import |
| `examples/CSV_Examples/milestonedata.csv` | Bulk milestone import |

### 1.5 Test Users Required

| User | Role | Purpose |
|---|---|---|
| Admin employee | `admin = 1` in DB | Tests requiring write access |
| Non-admin employee | `admin = 0` in DB | Permission-denial tests |

---

## 2. Navigation — `select` and `list`

Command aliases: `select` = `sl` = `cd`

### N-01 — Navigate to root

**Precondition:** Anywhere in the hierarchy (e.g., inside a project).
**Input:** `sl ~`
**Expected:** Prompt changes to `[osoem] ~/>`

---

### N-02 — Navigate up one level

**Precondition:** Inside `~/Projects/BuildingProject/`.
**Input:** `sl ..`
**Expected:** Prompt changes to `[osoem] ~/Projects/>`

---

### N-03 — Select folder by name (case-insensitive)

**Precondition:** At `~/`.
**Input:** `sl projects` (lowercase)
**Expected:** Enters Projects folder. Prompt shows `[osoem] ~/Projects/>`. Folder name resolution is case-insensitive.

---

### N-04 — Select entity by name

**Precondition:** At `~/Projects/` with at least one project named `Building Project`.
**Input:** `sl "Building Project"`
**Expected:** Enters that project. Prompt shows `[osoem] ~/Projects/Building Project/>`

---

### N-05 — Select entity by numeric ID

**Precondition:** At `~/Projects/` with a project with ID `1`.
**Input:** `sl 1`
**Expected:** Enters the project with ID 1.

---

### N-06 — Select non-existent item

**Precondition:** At `~/Projects/`.
**Input:** `sl DoesNotExist`
**Expected:** Error message — `EntityNotFoundError` or `NavigationError`. Prompt remains at `~/Projects/>`.

---

### N-07 — Navigate into every folder type

**Precondition:** Full dataset loaded via `project.src`.
**Steps:** Navigate into each of the following folder types and verify the prompt updates:

| Path | Folder | Expected Prompt Segment |
|---|---|---|
| `~/` | `Organization` | `.../Organization/` |
| `~/Organization/` | `Departments` | `.../Departments/` |
| `~/Projects/` | `<Project>` | `.../BuildingProject/` |
| `~/Projects/<Project>/` | `Activity Categories` | `.../Activity Categories/` |
| `.../Activity Categories/<Category>/` | `<SubCategory>` | (subcategory name) |
| `.../SubCategory/<Activity>/` | `Tasks` | `.../Tasks/` |
| `.../Tasks/<Task>/` | `Tasks` | (subtasks folder) |
| `.../Tasks/<Task>/` | `Assignments` | `.../Assignments/` |
| `.../Activity/` | `Notes` | `.../Notes/` |
| `~/Projects/<Project>/` | `Artefact Types` | `.../Artefact Types/` |
| `.../Artefact Types/<Type>/` | `Artefacts` | `.../Artefacts/` |
| `.../Artefact Types/<Type>/` | `Fields` | `.../Fields/` |
| `.../Artefacts/<Artefact>/` | `Data` | `.../Data/` |
| `.../Artefacts/<Artefact>/` | `Associations` | `.../Associations/` |
| `~/Projects/<Project>/` | `Milestones` | `.../Milestones/` |
| `~/Projects/<Project>/` | `Team` | `.../Team/` |

**Expected:** Each folder navigable; prompt reflects the path.

---

### N-08 — Navigate to leaf node (cannot go further)

**Precondition:** Inside an `Assignments` folder with at least one assignment.
**Steps:** `sl <AssigneeName>` to enter the assignment (a leaf node of type `ASSIGNMENT` from which `Hours` are added, not navigated to).
**Expected:** Prompt shows assignment path. `ls` shows hours entries. Attempting `sl ..` returns to `Assignments`.

---

### N-09 — Full path string for deeply nested item

**Precondition:** Full dataset loaded.
**Steps:** Navigate to `~/Projects/Building Project/Activity Categories/Structural Engineering/Design Development/Prepare structural drawings/Tasks/Model preparation/Tasks/Prepare final 3D model/`.
**Expected:** Prompt reflects the full hierarchy path at each step.

---

### N-10 — Select `.` (current directory)

**Precondition:** At any location.
**Input:** `sl .`
**Expected:** No navigation occurs; prompt remains unchanged.

---

### N-11 — Multi-level path from root using `/`

**Precondition:** At any location.
**Input:** `sl ~/Projects/Building Project`
**Expected:** Navigates to the `Building Project` entity in one command. Prompt shows `.../Building Project/>`. Internally, `~` resets to root, then each `/`-separated segment is applied in sequence.

---

### N-12 — Multi-level relative path without `~`

**Precondition:** At `~/`.
**Input:** `sl Organization/Departments`
**Expected:** Navigates into the Departments folder directly. Prompt shows `.../Departments/>`.

---

### N-13 — `..` segments inside slash-separated path

**Precondition:** At `~/Projects/`.
**Input:** `sl ../Projects`
**Expected:** `..` moves up to ROOT, then `Projects` navigates back to the Projects folder. Final prompt: `.../Projects/>`.

---

## 3. List Command

Command aliases: `list` = `ls`

### L-01 — List at root

**Precondition:** At `~/`.
**Input:** `ls`
**Expected:** Output shows `Organization` and `Projects` as the two top-level folders.

---

### L-02 — Default list format (ID + name)

**Precondition:** At `~/Projects/` with at least one project.
**Input:** `ls`
**Expected:** Two-column table showing each project's ID and name. No extra columns.

---

### L-03 — Detailed list with `-a` flag

**Precondition:** At `~/Projects/<Project>/Activity Categories/<Category>/<SubCategory>/` with at least one activity.
**Input:** `ls -a`
**Expected:** Tabular output showing all activity fields: `activityid`, `activityname`, `activitymanagerid`, `plannedstart`, `plannedfinish`, `plannedhours`.

---

### L-04 — List empty folder

**Precondition:** At `~/Projects/<Project>/Milestones/` with no milestones added.
**Input:** `ls`
**Expected:** Empty output or a message indicating no items; no crash.

---

### L-05 — List after `add` reflects new item

**Precondition:** At `~/Projects/<Project>/Milestones/` with 0 milestones.
**Steps:**
1. `ls` → note 0 items
2. `ad "milestonename:Test Milestone"`
3. `ls`
**Expected:** New milestone `Test Milestone` appears in the list.

---

### L-06 — List after `remove` shows item absent

**Precondition:** At `~/Projects/<Project>/Milestones/` with a milestone named `Test Milestone`.
**Steps:**
1. `rm "Test Milestone"`
2. `ls`
**Expected:** `Test Milestone` no longer appears.

---

### L-07 — List count matches import count

**Precondition:** Fresh database, at `~/Projects/`.
**Steps:**
1. `im examples/CSV_Examples/projectdata.csv`
2. `ls`
**Expected:** Number of rows in output equals number of data rows in `projectdata.csv`.

---

## 4. Add Command

Command aliases: `add` = `ad`

Two invocation modes:
- **Interactive**: `ad` — prompts for each field interactively.
- **Inline (key-value)**: `ad "field1:val1,field2:val2"` — parses CSV-style key:value pairs.
- **Inline (positional)**: `ad val1,val2,...` — positional field order per entity type.

### 4.1 General Add Tests

#### A-01 — Add with valid inputs (interactive)

**Precondition:** At `~/Projects/` in admin mode.
**Input:** `ad` → enter `projectname: Test Project`, `projectno: PRJ-001`
**Expected:** `Successfully added 1 entry`. Item visible with `ls`.

---

#### A-02 — Add cancelled by user (ESC / Ctrl+C in interactive)

**Precondition:** At `~/Projects/` in admin mode.
**Input:** `ad` → press ESC or Ctrl+C during prompts
**Expected:** `UserCancelledError` — `Add operation cancelled.` printed. No record created. `ls` count unchanged.

---

#### A-03 — Add duplicate name

**Precondition:** At `~/Projects/` with a project named `Test Project`.
**Input:** `ad "projectname:Test Project,projectno:PRJ-002"`
**Expected:** `DuplicateEntryError` — `Duplicate Project found: 'Test Project'. Aborting.`

---

#### A-04 — Add with invalid date format

**Precondition:** At `.../SubCategory/` with a valid activity manager in the project team.
**Input:** `ad "activityname:Test Activity,activitymanagername:John Davis,plannedstart:2026/01/01,plannedfinish:2026-04-01,plannedhours:100"`
**Expected:** `ValidationError` — date format rejected.

---

#### A-05 — Add with missing required field

**Precondition:** At `~/Projects/` in admin mode.
**Input:** `ad "projectno:PRJ-003"` (missing `projectname`)
**Expected:** `ValidationError` with format help message: `Invalid format. Use: ad <projectname>,<projectno>`

---

#### A-06 — Key-value format with unknown field key

**Precondition:** At `~/Projects/` in admin mode.
**Input:** `ad "projectname:Test,unknownfield:abc"`
**Expected:** `InvalidFieldError` or `ValidationError` — unknown field rejected.

---

### 4.2 Entity-Specific Add Tests

#### A-10 — Add Department

**Precondition:** At `~/Organization/Departments/`.
**Input:** `ad "departmentname:Structural Engineering"`
**Expected:** `Successfully added 1 entry`. Department visible via `ls`.

---

#### A-11 — Add Employee

**Precondition:** At `~/Organization/Departments/<Department>/` in admin mode.
**Input:** `ad "firstname:Jane,lastname:Smith,email:jane.smith@test.com,password:securepass123"`
**Expected:** `Successfully added 1 entry`. Employee visible via `ls`. Password stored in database as SHA-256 hash (not plaintext). The `passwordsalt` column is non-empty.

---

#### A-12 — Add Employee with duplicate email

**Precondition:** An employee with `email = jane.smith@test.com` already exists.
**Input:** `ad "firstname:Bob,lastname:Jones,email:jane.smith@test.com,password:pass"`
**Expected:** `DatabaseError` — unique constraint violation on `email` column.

---

#### A-13 — Add Project

**Precondition:** At `~/Projects/`.
**Input:** `ad "projectname:Bridge Project,projectno:PRJ-2026-002"`
**Expected:** `Successfully added 1 entry`.

---

#### A-14 — Add Activity Category

**Precondition:** At `~/Projects/<Project>/Activity Categories/`.
**Input:** `ad "categoryname:Civil Engineering"`
**Expected:** `Successfully added 1 entry`.

---

#### A-15 — Add Activity Subcategory

**Precondition:** At `.../Activity Categories/<Category>/`.
**Input:** `ad "subcategoryname:Foundation Design"`
**Expected:** `Successfully added 1 entry`.

---

#### A-16 — Add Activity with valid date range

**Precondition:** At `.../Activity Categories/<Category>/<SubCategory>/`. Activity manager is a project team member.
**Input:** `ad "activityname:Bridge Design,activitymanagername:John Davis,plannedstart:2026-03-01,plannedfinish:2026-06-30,plannedhours:500"`
**Expected:** `Successfully added 1 entry`. Activity visible with `ls -a`.

---

#### A-17 — Add Activity with non-team-member as manager

**Precondition:** At `.../SubCategory/`. Employee exists but is NOT in the project team.
**Input:** `ad "activityname:Test,activitymanagername:<non-member name>,plannedstart:2026-01-01,plannedfinish:2026-02-01,plannedhours:100"`
**Expected:** `ValidationError` — `Employee '<name>' is not a member of the project team.`

---

#### A-18 — Add Task (top-level)

**Precondition:** At `.../Activity/<ActivityName>/Tasks/`.
**Input:** `ad "taskname:Review drawings"`
**Expected:** `Successfully added 1 entry`.

---

#### A-19 — Add Sub-task (under a task)

**Precondition:** At `.../Tasks/<TaskName>/Tasks/`.
**Input:** `ad "taskname:Check dimensions"`
**Expected:** `Successfully added 1 entry`. Subtask linked to parent task via `parenttaskid`.

---

#### A-20 — Add Task at wrong location

**Precondition:** At `.../Activity/<ActivityName>/` (not in the Tasks folder).
**Input:** `ad "taskname:Test Task"`
**Expected:** `NavigationError` — `Cannot add an item here. Navigate to Tasks folder first.`

---

#### A-21 — Add Activity Note

**Precondition:** At `.../Activity/<ActivityName>/Notes/`. Author is a project team member.
**Input:** `ad "username:John Davis,note:Kickoff meeting completed.,notedate:2026-02-01"`
**Expected:** `Successfully added 1 entry`.

---

#### A-22 — Add Task Assignment

**Precondition:** At `.../Tasks/<Task>/Assignments/`. Assignee is a project team member.
**Input:** `ad "username:Sarah Thompson,assigneddate:2026-03-01"`
**Expected:** `Successfully added 1 entry`.

---

#### A-23 — Add Assignment with non-team-member

**Precondition:** At `.../Tasks/<Task>/Assignments/`. Employee exists but is not in the project team.
**Input:** `ad "username:<non-member name>,assigneddate:2026-03-01"`
**Expected:** `ValidationError` — not a project team member.

---

#### A-24 — Book Hours

**Precondition:** At `.../Assignments/<Assignment>/` (inside an assignment).
**Input:** `ad "bookeddate:2026-03-05,hours:8"`
**Expected:** `Successfully added 1 entry`.

---

#### A-25 — Add Artefact Type

**Precondition:** At `~/Projects/<Project>/Artefact Types/`.
**Input:** `ad "artefactname:Drawing,artefactdescription:Engineering drawings"`
**Expected:** `Successfully added 1 entry`.

---

#### A-26 — Add Artefact Data Field (text type)

**Precondition:** At `.../Artefact Types/<Type>/Fields/`.
**Input:** `ad "fieldtitle:Drawing Number,valuetype:0,maximumlength:50,maximumvalue:0,minimumvalue:0"`
**Expected:** `Successfully added 1 entry`. `valuetype=0`, `maximumlength=50`, `maximumvalue` and `minimumvalue` stored as NULL.

---

#### A-27 — Add Artefact Data Field (numeric type)

**Precondition:** At `.../Artefact Types/<Type>/Fields/`.
**Input:** `ad "fieldtitle:Number of Pages,valuetype:1,maximumlength:0,maximumvalue:500,minimumvalue:1"`
**Expected:** `Successfully added 1 entry`. `valuetype=1`, `maximumvalue=500`, `minimumvalue=1`, `maximumlength` stored as NULL.

---

#### A-28 — Add Artefact

**Precondition:** At `.../Artefact Types/<Type>/Artefacts/`. Owner is a project team member.
**Input:** `ad "artefacttitle:Foundation Plan S-101,artefactownername:Michael Chen"`
**Expected:** `Successfully added 1 entry`.

---

#### A-29 — Add Artefact Data Item

**Precondition:** At `.../Artefacts/<Artefact>/Data/`. Artefact data fields defined for this type.
**Input:** `ad "fieldname:Drawing Number,value:S-101"`
**Expected:** `Successfully added 1 entry`. Value linked to the correct `artefactdatafieldid`.

---

#### A-30 — Add ArtefactToActivityLink (valid ratio)

**Precondition:** At `.../Artefacts/<Artefact>/Associations/`. Activities exist in the project.
**Input:** `ad "activityname:Prepare structural drawings,ratio:0.50"`
**Expected:** `Successfully added 1 entry`.

---

#### A-31 — Add ArtefactToActivityLink (total ratio exceeds 1)

**Precondition:** Artefact already has a link with `ratio=0.75`.
**Input:** `ad "activityname:Foundation design,ratio:0.50"` (0.75 + 0.50 = 1.25)
**Expected:** `ValidationError` or `DatabaseError` — ratio sum constraint violated.

---

#### A-32 — Add Milestone

**Precondition:** At `~/Projects/<Project>/Milestones/`.
**Input:** `ad "milestonename:Drawing Approval"`
**Expected:** `Successfully added 1 entry`.

---

#### A-33 — Add Milestone Step (valid ratio)

**Precondition:** At `.../Milestones/<Milestone>/`.
**Input:** `ad "milestonestepname:Discipline Check,progressratio:0.50"`
**Expected:** `Successfully added 1 entry`.

---

#### A-34 — Add Milestone Step with ratio out of range

**Precondition:** At `.../Milestones/<Milestone>/`.
**Input:** `ad "milestonestepname:Bad Step,progressratio:1.50"`
**Expected:** `ValidationError` — `progressratio` must be between 0 and 1.

---

#### A-35 — Add Project Team Member

**Precondition:** At `~/Projects/<Project>/Team/`. Employee exists.
**Input:** `ad "employeename:Christopher Lee,role:1"`
**Expected:** `Successfully added 1 entry`.

---

#### A-36 — Add MilestoneToArtefactLink

**Precondition:** At `.../Associations/<ArtefactActivityLink>/`. Milestone steps exist.
**Input:** `ad "milestonestepname:Discipline Check,completedbyname:Michael Chen,completiondate:2026-03-15"`
**Expected:** `Successfully added 1 entry`.

---

#### A-37 — Add at ROOT or folder node

**Precondition:** At `~/` (ROOT).
**Input:** `ad "name:test"`
**Expected:** `NavigationError` — `Cannot add an item here.`

---

### 4.3 Inline (Positional) CSV Add

#### A-38 — Positional CSV add (department)

**Precondition:** At `~/Organization/Departments/`.
**Input:** `ad "Mechanical Engineering"`
**Expected:** `Successfully added 1 entry`. Department created.

---

#### A-39 — Positional CSV add (employee)

**Precondition:** At `~/Organization/Departments/<Department>/`.
**Input:** `ad "Tom,Jones,tom.jones@test.com,pass123"`
**Expected:** `Successfully added 1 entry`. Employee created with hashed password.

---

## 5. Remove Command

Command aliases: `remove` = `rm`

### R-01 — Remove by name

**Precondition:** At `~/Projects/` with a project named `Test Project`.
**Input:** `rm "Test Project"`
**Expected:** Item removed. `ls` no longer shows `Test Project`.

---

### R-02 — Remove by numeric ID

**Precondition:** At `~/Projects/` with a project having ID `2`.
**Input:** `rm 2`
**Expected:** Project with ID 2 removed. `ls` confirms absence.

---

### R-03 — Remove non-existent item

**Precondition:** At `~/Projects/` with no project named `Phantom Project`.
**Input:** `rm "Phantom Project"`
**Expected:** `EntityNotFoundError` — `Project not found with name 'Phantom Project'.`

---

### R-04 — Remove with no argument

**Precondition:** At `~/Projects/`.
**Input:** `rm`
**Expected:** `NavigationError` — `Please provide an item name or ID to remove.`

---

### R-05 — Cascade delete: remove project with child data

**Precondition:** A project with categories, activities, tasks, and artefacts.
**Input:** `rm "<ProjectName>"` from `~/Projects/`
**Expected:** Either all child records are cascade-deleted (if FK `ON DELETE CASCADE` is set in schema), or a `DatabaseError` / `ConstraintViolationError` is raised preventing deletion.

---

### R-06 — Remove project team member

**Precondition:** At `~/Projects/<Project>/Team/` with a team member.
**Input:** `rm <MemberNameOrId>`
**Expected:** Team membership record removed. The employee still exists at `~/Organization/Departments/<Department>/`. The project still exists.

---

### R-07 — Remove activity category with subcategories

**Precondition:** At `.../Activity Categories/` with a category that has subcategories.
**Input:** `rm <CategoryName>`
**Expected:** Either cascades to delete subcategories and activities, or raises a `DatabaseError` / `ConstraintViolationError`.

---

### R-08 — Remove employee referenced as activity manager

**Precondition:** Employee is the `activitymanagerid` in at least one activity.
**Input:** `rm <EmployeeName>` from their department.
**Expected:** `DatabaseError` — foreign key constraint prevents deletion.

---

## 6. Set Command

Command aliases: `set` = `st`

Syntax: `st <fieldname> <value>`

### S-01 — Set valid text field on entity

**Precondition:** Navigated inside a project (type `PROJECT`).
**Input:** `st projectname "Updated Project Name"`
**Expected:** `Successfully updated project details.` `ls` at parent shows new name.

---

### S-02 — Set date field (valid format)

**Precondition:** Navigated inside an activity (type `ACTIVITY`).
**Input:** `st plannedstart 2026-05-01`
**Expected:** `Successfully updated activity details.` Date updated in DB.

---

### S-03 — Set date field (invalid format)

**Precondition:** Navigated inside an activity.
**Input:** `st plannedstart 01/05/2026`
**Expected:** `ValidationError` — date format rejected.

---

### S-04 — Set unknown field name

**Precondition:** Navigated inside a project.
**Input:** `st nonexistentfield somevalue`
**Expected:** `InvalidFieldError` — field name not recognized for this entity.

---

### S-05 — Set numeric field to non-numeric value

**Precondition:** Navigated inside an activity.
**Input:** `st plannedhours notanumber`
**Expected:** `ValidationError` — numeric field validation failure.

---

### S-06 — Set field at a folder node

**Precondition:** At `~/Projects/Activity Categories/` (folder node type `ACTIVITIES_FOLDER`).
**Input:** `st name X`
**Expected:** `NavigationError` — `Cannot edit an item here.`

---

### S-07 — Set field at ROOT

**Precondition:** At `~/`.
**Input:** `st name test`
**Expected:** `NavigationError` — `Cannot edit an item here.`

---

### S-08 — Set employee password

**Precondition:** Navigated inside an employee (type `EMPLOYEE`).
**Input:** `st password newpassword123`
**Expected:** `Successfully updated employee details.` New password stored as SHA-256 hash with a new salt in the DB. Old hash is replaced.

---

### S-09 — Set milestone step progressratio

**Precondition:** Navigated inside a milestone step (type `MILESTONE_STEP`).
**Input:** `st progressratio 0.75`
**Expected:** `Successfully updated milestone step details.`

---

### S-10 — Set artefact-to-activity link ratio

**Precondition:** Navigated inside an `ArtefactToActivityLink` entity.
**Input:** `st ratio 0.60`
**Expected:** `Successfully updated artefact to activity link details.`

---

## 7. Import Command

Command aliases: `import` = `im`

Syntax: `im <filepath>`

### I-01 — Import valid CSV (employees)

**Precondition:** At `~/Organization/Departments/<Department>/` (e.g., ID 6). DB has the referenced `departmentid`.
**Input:** `im examples/CSV_Examples/employeedata.csv`
**Expected:** `Successfully added 16 entries` (matching the 16 data rows in the file). All employees visible via `ls`.

---

### I-02 — Import valid CSV (projects)

**Precondition:** At `~/Projects/`.
**Input:** `im examples/CSV_Examples/projectdata.csv`
**Expected:** `Successfully added N entries` equal to the row count in `projectdata.csv`.

---

### I-03 — Import with duplicate rows (idempotency)

**Precondition:** `employeedata.csv` already imported once. At same department path.
**Input:** `im examples/CSV_Examples/employeedata.csv` (re-import)
**Expected:** `DuplicateEntryError` listing the duplicate names. No partial import. Existing records unchanged.

---

### I-04 — Import with missing required column

**Precondition:** At `~/Projects/`. Create a CSV file `bad_project.csv` with header `projectno` only (missing `projectname`).
**Input:** `im bad_project.csv`
**Expected:** `CsvParseError` or `ValidationError` — required column missing.

---

### I-05 — Import with extra unknown column

**Precondition:** At `~/Projects/`. Create a CSV file with headers `projectname,projectno,unknownfield`.
**Input:** `im extra_col.csv`
**Expected:** `InvalidFieldError` or graceful warning; unknown column rejected or ignored depending on implementation.

---

### I-06 — Import at wrong location (entity type mismatch)

**Precondition:** At `~/Projects/` (projects list).
**Input:** `im examples/CSV_Examples/employeedata.csv` (employee CSV at projects path)
**Expected:** `ValidationError` or `CsvParseError` — field names from the CSV do not match expected fields for projects.

---

### I-07 — Import resolves name-based foreign keys

**Precondition:** At `~/Projects/<Project>/Activity Categories/<Category>/<SubCategory>/`. Activity managers already exist as employees in the project team.
**Input:** `im examples/CSV_Examples/activitydata.csv`
**Expected:** Activities created with correct `activitymanagerid` resolved from the name in the CSV.

---

### I-08 — Import with invalid date in a row

**Precondition:** At a subcategory path. Create `bad_activity.csv` with one row containing `plannedstart=not-a-date`.
**Input:** `im bad_activity.csv`
**Expected:** `ValidationError` or `CsvParseError` with line number context.

---

### I-09 — Import non-existent file

**Precondition:** Any valid path in admin mode.
**Input:** `im /tmp/does_not_exist.csv`
**Expected:** `FileIOError` — `Could not open file '/tmp/does_not_exist.csv'`

---

### I-10 — Import milestones

**Precondition:** At `~/Projects/<Project>/Milestones/`.
**Input:** `im examples/CSV_Examples/milestonedata.csv`
**Expected:** `Successfully added N entries`.

---

### I-11 — Import milestone steps

**Precondition:** At `.../Milestones/<Milestone>/`.
**Input:** `im examples/CSV_Examples/milestonestepdata.csv`
**Expected:** `Successfully added N entries`. All `progressratio` values within [0, 1].

---

### I-12 — Import with `-i` flag ignores duplicates

**Precondition:** At `~/Organization/Departments/<Department>/` with employees already imported once.
**Input:** `im -i examples/CSV_Examples/employeedata.csv` (re-import of existing data)
**Expected:** Completes without error. No new records added. Existing records unchanged. The `-i` (ignore duplicates) flag suppresses `DuplicateEntryError`.

---

## 8. Export Command

Command aliases: `export` = `ex`

Syntax: `ex <filepath>`

### E-01 — Export at an entity list location

**Precondition:** At `~/Projects/` with at least two projects. Admin or non-admin (export does not require admin).
**Input:** `ex /tmp/projects_export.csv`
**Expected:** File created at `/tmp/projects_export.csv`. File contains a header row and one data row per project. Data matches `ls -a` output.

---

### E-02 — Export then import to empty DB (round-trip)

**Precondition:** DB has employees under a department.
**Steps:**
1. Navigate to the department. `ex /tmp/employees_export.csv`.
2. Reset the DB to schema-only state.
3. Navigate to the same department path. `im /tmp/employees_export.csv`.
4. `ls`
**Expected:** Row count after import matches row count before export.

---

### E-03 — Export at a folder node

**Precondition:** At `~/Projects/<Project>/` (a project entity, not a list).
**Input:** `ex /tmp/test.csv`
**Expected:** Either exports the contained sub-items or raises a `NavigationError`. Behavior should be documented.

---

### E-04 — Export to existing file path

**Precondition:** `/tmp/existing.csv` already exists.
**Input:** `ex /tmp/existing.csv`
**Expected:** File is overwritten with new data, or a `FileIOError` is raised. Behavior should be documented.

---

### E-05 — Export activities (with all fields)

**Precondition:** At `.../Activity Categories/<Category>/<SubCategory>/` with activities loaded.
**Input:** `ex /tmp/activities.csv`
**Expected:** CSV contains columns: `activityname`, `activitymanagerid`, `plannedstart`, `plannedfinish`, `plannedhours` (or name variants). Row count matches `ls` output.

---

### E-06 — Export departments from `DEPARTMENTS_FOLDER`

**Precondition:** At `~/Organization/Departments/`.
**Input:** `ex /tmp/departments.csv`
**Expected:** File created with header `departmentname`. One row per department. Row count matches `SELECT COUNT(*) FROM Departments`.

---

### E-07 — Export team members from `TEAM_FOLDER`

**Precondition:** At `~/Projects/<Project>/Team/`.
**Input:** `ex /tmp/team.csv`
**Expected:** File created with header `firstname,lastname,email,role`. Row count matches project team member count.

---

### E-08 — Export artefact data from `DATA_FOLDER`

**Precondition:** At `.../Artefacts/<Artefact>/Data/`.
**Input:** `ex /tmp/artefact_data.csv`
**Expected:** File created with header `fieldtitle,value`. One row per populated data field.

---

### E-09 — Export activity associations from `ASSOCIATIONS_FOLDER`

**Precondition:** At `.../Artefacts/<Artefact>/Associations/`.
**Input:** `ex /tmp/associations.csv`
**Expected:** File created with header `activityname,ratio`. One row per activity link.

---

### E-10 — Export milestone links from `ARTEFACT_TO_ACTIVITY_LINK`

**Precondition:** At `.../Associations/<ActivityName>/` (inside an artefact-to-activity link).
**Input:** `ex /tmp/milestone_links.csv`
**Expected:** File created with header `milestonestepname,completedbyid,completiondate`. One row per milestone link.

---

### E-11 — Export task assignments from `ASSIGNMENTS_FOLDER`

**Precondition:** At `.../Tasks/<Task>/Assignments/`.
**Input:** `ex /tmp/assignments.csv`
**Expected:** File created with header `userid,assigneddate,closedate`. One row per assignment.

---

### E-12 — Export hours bookings from `ASSIGNMENT`

**Precondition:** At `.../Assignments/<Assignment>/` (inside a specific assignment).
**Input:** `ex /tmp/hours.csv`
**Expected:** File created with header `bookeddate,hours`. One row per hours booking.

---

### E-13 — Export from `DEPARTMENT` filters employees by department

**Precondition:** Two departments exist, each with a different number of employees.
**Steps:**
1. Navigate to Department 1. `ex /tmp/dept1.csv`.
2. Navigate to Department 2. `ex /tmp/dept2.csv`.
**Expected:** Each file contains only the employees in that department. Row counts match `SELECT COUNT(*) FROM Employees WHERE departmentid=<id>`. The two row counts differ.

---

## 9. Authentication and Authorization

### AU-01 — Login with valid admin credentials

**Precondition:** An employee exists with `admin = 1` and known password.
**Input:** `./osoem -u admin@test.com` → enter correct password
**Expected:** `Logged in as: <FirstName> <LastName> (admin@test.com)` and `Role: Admin`. Full write access.

---

### AU-02 — Login with valid non-admin credentials

**Precondition:** An employee exists with `admin = 0` and known password.
**Input:** `./osoem -u user@test.com` → enter correct password
**Expected:** `Logged in as: <FirstName> <LastName>` and `Role: User`. Read-only access.

---

### AU-03 — Login with wrong password

**Precondition:** Employee `admin@test.com` exists.
**Input:** `./osoem -u admin@test.com` → enter wrong password
**Expected:** `Authentication failed. Invalid email or password.` Process exits with return code 1.

---

### AU-04 — Login with non-existent email

**Precondition:** No employee with email `nobody@test.com`.
**Input:** `./osoem -u nobody@test.com` → enter any password
**Expected:** `Authentication failed. Invalid email or password.` Process exits with return code 1.

---

### AU-05 — Non-admin tries `add`

**Precondition:** Logged in as non-admin user. At `~/Projects/`.
**Input:** `ad "projectname:X,projectno:Y"`
**Expected:** `PermissionError` — `Permission denied. Admin access required.`

---

### AU-06 — Non-admin tries `remove`

**Precondition:** Logged in as non-admin. At `~/Projects/` with a project.
**Input:** `rm <projectname>`
**Expected:** `PermissionError` — `Permission denied. Admin access required.`

---

### AU-07 — Non-admin tries `set`

**Precondition:** Logged in as non-admin. Inside a project entity.
**Input:** `st projectname "New Name"`
**Expected:** `PermissionError` — `Permission denied. Admin access required.`

---

### AU-08 — Non-admin tries `import`

**Precondition:** Logged in as non-admin. At `~/Projects/`.
**Input:** `im examples/CSV_Examples/projectdata.csv`
**Expected:** `PermissionError` — `Permission denied. Admin access required.`

---

### AU-09 — Non-admin can use `list` and `select`

**Precondition:** Logged in as non-admin.
**Steps:** `ls`, `sl Projects`, `sl <Project>`, `ls`
**Expected:** All navigation and listing works without error.

---

### AU-10 — Non-admin can use `export`

**Precondition:** Logged in as non-admin. At `~/Projects/` with projects loaded.
**Input:** `ex /tmp/proj.csv`
**Expected:** Export succeeds. No `PermissionError`.

---

### AU-11 — No `-u` flag (current default behavior)

**Precondition:** `UserContext` defaults `isAdmin = true` (line 19 of `auth.h`).
**Input:** `./osoem` (no flags)
**Expected:** Admin access is active. `add`, `remove`, `set`, `import` all permitted. No login prompt.
> **Note:** This is a development-mode default. The `/*false*/` comment in `auth.h` indicates this should eventually default to `false` (read-only). When that change is made, this test should be updated to expect `PermissionError` for write operations.

---

### AU-12 — Password stored as hash, not plaintext

**Precondition:** Admin mode.
**Steps:**
1. `ad` an employee with password `mysecretpass`.
2. Inspect the `Employees` table in MySQL: `SELECT password, passwordsalt FROM Employees WHERE email = '...'`
**Expected:** `passwordsalt` is a 64-character hex string. `password` is a 64-character hex string (SHA-256 output). Neither column contains `mysecretpass`.

---

## 10. Batch Execution

### B-01 — Run valid `.src` file

**Precondition:** Fresh database with schema applied. Departments and employees seeded via `departments.src`.
**Input:** `./osoem examples/Residential_Building_Project/project.src`
**Expected:** All commands execute sequentially. Each line is echoed as `[osoem: examples/Residential_Building_Project/project.src, Line N] <path>> <command>`. Final output: `Finished executing commands from file.` All data visible in interactive mode.

---

### B-02 — Comment lines (`#`) are ignored

**Precondition:** Script file contains lines starting with `#`.
**Expected:** Lines starting with `#` produce no output and no command execution. No `CommandError`.

---

### B-03 — Empty lines are skipped

**Precondition:** Script file contains blank lines between commands.
**Expected:** Blank lines produce no output and no command execution. Navigation state is unaffected.

---

### B-04 — Script stops at first error

**Precondition:** Create a script `bad.src` with:
```
cd ~
cd Projects
rm "Nonexistent Project"
cd Organization
```
**Input:** `./osoem bad.src`
**Expected:** Error at `rm "Nonexistent Project"` (EntityNotFoundError) is printed. Execution stops. `cd Organization` is never run. Return code is 0 (the error is caught, not fatal), but output shows `Error: ...`.

---

### B-05 — Batch mode is always admin

**Precondition:** Run without `-u` flag.
**Input:** `./osoem examples/Residential_Building_Project/project.src`
**Expected:** All `add`/`remove`/`set`/`import` commands in the script execute without `PermissionError`. Confirmed by `batchExecute` always calling `execute(..., isAdmin=1)`.

---

### B-06 — Navigation state persists across script lines

**Precondition:** Script navigates deep into hierarchy using `cd` and then uses `ad`.
**Expected:** State from each `cd` line is preserved for subsequent lines. The `ad` command operates at the correct nested path.

---

### B-07 — Run non-existent script file

**Precondition:** File `no_such_file.src` does not exist.
**Input:** `./osoem no_such_file.src`
**Expected:** `FileIOError` — `Could not open file 'no_such_file.src'`. Process exits.

---

## 11. Input Parsing and Edge Cases

### P-01 — Quoted string with spaces

**Precondition:** At `~/Projects/` with a project named `Building Project`.
**Input:** `sl "Building Project"`
**Expected:** Successfully navigates into `Building Project`.

---

### P-02 — Escaped quotes inside quoted string

**Precondition:** At `~/Organization/Departments/`.
**Input:** `ad "departmentname:O\'Brien Engineering"`
**Expected:** Department created with name `O'Brien Engineering` (single quote preserved via escape).

---

### P-03 — Case-insensitive command recognition

**Precondition:** At any location.
**Input (three separate tests):** `LIST`, `List`, `list`
**Expected:** All three recognized as the `list` command and produce identical output.

---

### P-04 — Case-insensitive folder name navigation

**Precondition:** At `~/Projects/<Project>/`.
**Input (tests):** `sl team`, `sl TEAM`, `sl Team`
**Expected:** All navigate to the Team folder (folder name matching is case-insensitive via `toLower`).

---

### P-05 — Unknown command

**Precondition:** Any location.
**Input:** `foobar`
**Expected:** `CommandError` — `Unknown command.` or `Unknown command: foobar`

---

### P-06 — Tab completion at root

**Precondition:** Interactive mode, at `~/`.
**Input:** `sl ` followed by TAB
**Expected:** Completion suggests `Organization` and `Projects`.

---

### P-07 — EOF / Ctrl+D in interactive mode

**Precondition:** Interactive mode running.
**Input:** Ctrl+D
**Expected:** Program exits gracefully. No crash. Returns exit code 0.

---

### P-08 — Empty input line

**Precondition:** Interactive mode.
**Input:** Press Enter on an empty line
**Expected:** No action. Prompt re-displayed. No error.

---

### P-09 — `exit` and `quit` commands

**Precondition:** Interactive mode.
**Input (two tests):** `exit`, `quit`
**Expected:** Both commands terminate the interactive session gracefully. Return code 0.

---

### P-10 — Single-quoted strings

**Precondition:** At `~/Projects/`.
**Input:** `sl 'Building Project'`
**Expected:** Single quotes treated the same as double quotes; navigates into `Building Project`.

---

### P-11 — Multiple spaces between tokens

**Precondition:** At any location.
**Input:** `ls   -a` (extra spaces)
**Expected:** Parsed correctly as `ls -a`. Full tabular output displayed.

---

### P-12 — Name starting with `.` is rejected

**Precondition:** At `~/Organization/Departments/`.
**Input:** `ad "departmentname:.hidden"`
**Expected:** `ValidationError` — names may not start with `.` (reserved for navigation operators).

---

### P-13 — Name containing `/` is rejected

**Precondition:** At `~/Projects/`.
**Input:** `ad "projectname:my/project,projectno:PRJ-X"`
**Expected:** `ValidationError` — `/` is a reserved path separator character and is not allowed in entity names.

---

## 12. Data Integrity

### DI-01 — ArtefactToActivityLink ratio sum constraint

**Precondition:** An artefact is linked to Activity A with `ratio = 0.80`.
**Steps:** Try to add another link for the same artefact to Activity B with `ratio = 0.30` (total would be 1.10).
**Expected:** `ValidationError` or `DatabaseError` — total ratio cannot exceed 1.0.

---

### DI-02 — MilestoneStep progressratio bounds

**Precondition:** At `.../Milestones/<Milestone>/`.
**Steps (two tests):**
  a. `ad "milestonestepname:Too High,progressratio:1.01"` — expect `ValidationError`
  b. `ad "milestonestepname:Negative,progressratio:-0.10"` — expect `ValidationError`
**Expected:** Both rejected with `ValidationError`.

---

### DI-03 — Hours value must be positive

**Precondition:** Inside an `Assignment` node.
**Input:** `ad "bookeddate:2026-03-01,hours:-5"`
**Expected:** `ValidationError` — negative hours rejected.

---

### DI-04 — Activity manager must be a project team member

**Precondition:** Employee exists in the system but is not in `ProjectTeamMembers` for the current project.
**Steps:** Try to add an activity using that employee as manager.
**Expected:** `ValidationError` — `Employee '<name>' is not a member of the project team.`

---

### DI-05 — Foreign key: delete employee referenced as activity manager

**Precondition:** Employee is the `activitymanagerid` for at least one activity.
**Steps:** Navigate to employee's department. `rm <EmployeeName>`.
**Expected:** `DatabaseError` — foreign key constraint violation. Employee not deleted.

---

### DI-06 — Artefact owner must be a project team member

**Precondition:** At `.../Artefact Types/<Type>/Artefacts/`. Employee is not in project team.
**Input:** `ad "artefacttitle:Test Drawing,artefactownername:<non-member>"`
**Expected:** `ValidationError` — not a project team member.

---

### DI-07 — Note author must be a project team member

**Precondition:** At `.../Activity/<Activity>/Notes/`. Employee is not in the project team.
**Input:** `ad "username:<non-member>,note:Test note,notedate:2026-01-01"`
**Expected:** `ValidationError` — not a project team member.

---

### DI-08 — Task assignment must be to a project team member

**Precondition:** At `.../Tasks/<Task>/Assignments/`. Employee exists but not in project team.
**Input:** `ad "username:<non-member>,assigneddate:2026-01-01"`
**Expected:** `ValidationError` — not a project team member.

---

### DI-09 — Milestone completion by non-team member

**Precondition:** At `.../Associations/<Link>/`. Employee not in project team.
**Input:** `ad "milestonestepname:Discipline Check,completedbyname:<non-member>,completiondate:2026-03-01"`
**Expected:** `ValidationError` — not a project team member.

---

### DI-10 — Entity not found errors use correct entity name

**Precondition:** Try to add an artefact referencing a non-existent artefact type ID.
**Expected:** `EntityNotFoundError` — `Artefact type with ID <X> not found.`

---

## 13. Help Command

Command aliases: `help` = `h`

Syntax: `help [topic]`

### H-01 — `help` with no arguments

**Precondition:** Any location.
**Input:** `help`
**Expected:** General command reference printed. Lists all commands: `list`, `select`, `add`, `remove`, `set`, `import`, `export`, `help`, `exit`. Each command shows its alias(es).

---

### H-02 — `help list`

**Precondition:** Any location.
**Input:** `help list` (or `help ls`)
**Expected:** List command documentation printed. Includes `-a` flag description.

---

### H-03 — `help select`

**Precondition:** Any location.
**Input:** `help select` (or `help cd`, `help sl`)
**Expected:** Navigation documentation printed. Must mention `~`, `..`, and `~/a/b/c` multi-level path syntax.

---

### H-04 — `help add`

**Precondition:** Any location.
**Input:** `help add` (or `help ad`)
**Expected:** Add command documentation printed. Includes interactive mode and inline CSV mode.

---

### H-05 — `help remove`

**Precondition:** Any location.
**Input:** `help remove` (or `help rm`)
**Expected:** Remove command documentation printed.

---

### H-06 — `help set`

**Precondition:** Any location.
**Input:** `help set` (or `help st`)
**Expected:** Set command documentation printed.

---

### H-07 — `help import`

**Precondition:** Any location.
**Input:** `help import` (or `help im`)
**Expected:** Import command documentation printed. Must mention `-r` (recursive) and `-i` (ignore duplicates) flags.

---

### H-08 — `help export`

**Precondition:** Any location.
**Input:** `help export` (or `help ex`)
**Expected:** Export command documentation printed.

---

### H-09 — `help` with unknown topic

**Precondition:** Any location.
**Input:** `help unknowncommand`
**Expected:** Fallback message printed (e.g., unknown topic or general help shown). No exception thrown. Program continues.

---

### H-10 — Short alias `h` works identically to `help`

**Precondition:** Any location.
**Input:** `h` then `h list`
**Expected:** Output identical to `help` and `help list` respectively.

---

## Critical Files Reference

| File | Purpose |
|---|---|
| `src/commands/execute.cpp` | Command dispatcher; routes `list`, `select`, `add`, `remove`, `set`, `import`, `export`, `help` |
| `src/commands/add.cpp` | `add` logic for all 19+ entity types; interactive and inline modes |
| `src/commands/remove.cpp` | `remove` logic; name/ID lookup per entity type |
| `src/commands/list.cpp` | `list` rendering; default and `-a` modes |
| `src/commands/select.cpp` | Navigation logic; folder resolution; `~`, `..`, `~/a/b/c` multi-level path, name, ID |
| `src/commands/set.cpp` | Field update logic for all editable entity types |
| `src/commands/importcsv.cpp` | CSV import; delegates to `add` functions; `-r` recursive, `-i` ignore duplicates |
| `src/commands/exportcsv.cpp` | CSV export for all location types |
| `src/commands/help.h` | Help command declaration |
| `src/commands/help.cpp` | Help command implementation with per-topic documentation |
| `src/utils/auth.cpp` | SHA-256 password hashing, salt generation, authentication |
| `src/utils/utils.cpp` | Validation helpers (`isNumber`, `toLower`, `startsWithDot`, `containsReservedChars`, etc.) |
| `src/utils/csv_parser.cpp` | CSV parsing; key:value and positional modes |
| `src/exceptions/exceptions.h` | All exception types |
| `src/data/path.cpp` | Path state management; `move_forward`, `move_back`, `reset` |
| `src/utils/auth.h` | `UserContext` struct; note `isAdmin` defaults to `true` in development |
| `config/db_config.h` | Database connection parameters |
| `../database/reset_database.sql` | Resets DB to seeded state |
| `../database/create_database.sql` | Creates empty DB with schema only |
| `../database/schema/schema.sql` | Full database schema |
| `examples/CSV_Examples/*.csv` | Sample import files for all entity types |
| `examples/Organization_Examples/departments.src` | Batch script: creates departments and employees |
| `examples/Residential_Building_Project/project.src` | Batch script: creates a complete project with all entity types |
