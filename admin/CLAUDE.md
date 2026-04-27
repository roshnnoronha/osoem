# Osoem - Admin tool

## Description
The admin tool is a command line tool that is used to perform adminstrative tasks, such as creating and editing data, on the osoem database.

## Osoem Database
The schema for the osoem database is described in the following file:
'../database/schema/schema.sql'

## Building

To build the admin tool (without compiling test cases):
```
cmake --build build/ --target=osoem_admin
```

## Implementation:
The tool is an interactive command line tool using which a user can navigate through the data in a heirarchy folder like structure. The folder structure of would look something like this:
root
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
The root folder is represented using the `~` symbol. Items appended with '**[Folder]**' in the above heirarchy represent notional containers similar to a folder in a file structure, that is used to sort the data items.

The program prompt will display the current location the user is in the heirarchy. For example:
[osoem] ~/projects/categories/subcategories/activities/tasks/subtasks>

The user can navigate through this structure using commands:
1. `list`
2. `select` 

The user will also be able to modify data using the following commands:
1. `add` 
2. `remove` 
3. `import` 
4. `export` 
5. `set`

To batch run a series of commands the user can call the program with the filename as an argument that contains the commands to run.

### Commands
**`list` (or `ls`)** 
The `list` command is used to list the items in the current folder. By default list will display the ID and NAME of the items in the current location. Using the `-a` flag will result in all the data displayed in a tabular format. 

**`select` (or `sl`, or `cd`)**
The `select` command is used to navigate to an item in the current folder. The syntax would be `sl <item_name>`. Using `sl ..` should take the user one step above in the heirarchy. `sl ~` will take the user to the root folder.

**`add` (or `ad`)**
Used to add a new item. The syntax to be used is: `ad <item_ name>`. The program will then prompt the user to enter all the required data for that item.

**`remove` (or `rm`)**
Used to remove an item. The syntax to be used is: `rm <item_name>`. 

**`import` (or `im`)**
Used to import data from a csv file.

**`export` (or `ex`)**
Used to export data to a csv file.

**`set` (or `st`)**
Used to edit a value for a field.
