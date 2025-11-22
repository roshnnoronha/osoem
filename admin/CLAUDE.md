# Osoem - Admin tool

The admin tool is a command line tool that is used to perform adminstrative task on the osoem database.

The tool is an interactive command line tool using which a user can navigate through the data in a heirarchy folder like structure. The folder structure of would look something like this:

root
    ├── Employees (data)
    └── Projects (data)
        ├── Activity (folder)
        │   └── ActivityCategories (data)
        │       └── ActivitySubcategories (data)
        │           └── Activities (data)
        │               ├── Tasks (data)
        │               │   └── Sub tasks (data)
        │               └── ActivityNotes (data)
        ├── Artefact (folder)
        │   ├── ArtefactTypes (data)
        │   │   └── Artefacts (data)
        │   │       └── ArtefactData (data)
        │   └── ArtefactFields (data)
        └── Milestones (folder)
            └── Milestones (data)
                └── MilestoneSteps (data)
                    └── MilestoneToArtefactLink (data)
Note that the items shown with data within parentheses represent data from the database, while items with folder within the parentheses represent notional items that is used to sort the items within a project.

The program prompt will display the current path. For example:
root/projects/categories/subcategories/activities/tasks/subtasks>

The user can navigate through this structure using commands:
ls/list -> Used to list the items in the current folder
sl/select -> Used to navigate to an item in the current folder. The syntax would be sl <item_name>. Using sl .. should take the user one step above in the heirarchy.

The user will also be able to add and remove items in a folder using the following commands:
ad/add -> Used to add a new item. The syntax to be used is: ad <item_ name>. The program will then prompt the user to enter all the required data for that item.
rm/remove -> Used to remove an item. The syntax to be used is: rm <item_name>. 


## Implementation:
The admin tool is developed in C++.
