[x] Departments table - create sample data
[x] Employees table - create sample data
[x] Projects table - create sample data
[x] Teams table - create sample data
[x] Categories table - create sample data
[x] Subcategories table - create sample data
[x] Activities table - create sample data
[x] ProjectTaskList table - create sample data
[x] ActivityTaskList table - create sample data
[x] Tasks table - create sample data
[x] Milestones table - create sample data
[x] MilestoneProgress table - create sample data
[x] UnitofMeasure table - create sample data
[x] ArtefactTypes table - create sample data
[x] Main login page - check username and password and set the session variable
[x] Dashboard - enable calling the details of the selected task
[x] Update the task page as per the qry_manage_activity.md file
[x] Artefacts table - create sample data
[x] Create a table for artefact data
[x] Create query for artifacts for the task details page
[x] Prepare the milestone schema
[x] Create the sql for the artefact details page
[x] Correct error on manage_activity.php
[x] Correct error on manage_artefact.php
[x] Add a back button for  artefact pages
[x] Create the sql for the milestone details
[x] Create the artifact details page
[x] Update the artefact progress based on milestones
[x] Show only latest revisions of artefacts
[x] Hours table - create sample data
[x] ArtefactRevisions table - create sample data
[x] Elements table - create sample data
[x] Dashboard - write query to show all available avtivities
[x] Create a file tasktree.php which takes an activity id as input and returns a task tree.
[x] In tasktree.php add the activity name at the top.
[x] In ./public/activity.php, move the section showing the users tasks under the task heirarchy and modify it to show only the open tasks
[x] In ./public/activity.php, correct the display error in the gantt chart. currently the timeline on the top does not align with the bars for the date.
[x] In ./public/activity.php, make the container divs for the bars to be transparent, make all the bars of the gantt chart black in colour, and all the text to be white in colour, and make the timeline container div to transparent.
[x] In ./public/actiivty.php. format the Artifact section so that it displays as a full with <div> under the activity and task details.
[x] In ./public/activity.php update the format of the hours bar graph to be similar to the gantt chart. The bars should be black with white text and there should be a Planned, Forecast and Actual title against the corresponding bar.
[x] In ./public/activity.php, make Activity notes into its own card. The formatting should be similar to the gannt chart card.
[x] In ./public/activity.php,Move notes to the end of the page and make it span accross the screen
[x] In ./public/activity.php, Make activity form fields to a separate card, activity details should not be editable
[x] In public/activity.php, put the task tree section into its own card, and give it the title "All Tasks"
[x] In public/activity.php, put the "Your open tasks" section into its own card, and give it the title "My Tasks"
[x] In public/activity.php, remove the "Details" against all the artefacts, make the table row clickable, navigating to the details of that artefact.
[x] In public/activity.php,check to ensure the alignment of the table columns are maintained accross all artefact types.
[x] In public/activity.php, change the colour of the artefact progress bar to match that of the gantt chart.
[x] In ./public/activity.php, the artefact type cards should be arranged in two columns within the Artefact card.
[x] In ./public/activity.php, remove the title row for the artefact table.
[x] In ./public/artefact.php, put "Artefact Details" and "Progress" into their own cards. The cards should have the same formatting as the cards in ./public/activity.php. Artefact details should not be editable.  
[x] In ./public/activity.php, against the "Tasks" section that displays the task tree include buttons to add and delete an tasks. These buttons should open a dialog box where the details like the task name and the parent task is provided. These buttons should only be available if the current user is the activity manager.
[x] In ./public/activity.php, against the "Tasks" section that displays the task tree include a button to assign an task. This button should open a dialog which displays a list of tasks that are currently assigned to the activity manager, and a list of users that the task can be assigned to. This button should only be available if the current user is the activity manager.
[x] In ./public/activity.php, against the "Activity" section that displays the activity details, add a button to edit the activity details. This should open a dialog box where the user can put the new values. This option should only be available to the Project manager.
[x] In ./public/activity.php, against each assigned task in the "My Tasks" section there should be a button to reassign the task . Reassign should open a dialog box that displays a list of users the task can be reassigned to.
[x] In ./public/activity.php, against each assigned task in the "My Tasks" section there should be a button to close the task. Closing the task should reassign the task to the activity manager if the current user is not the activity manager, and if the current user is the activity manager the task should be marked as closed by inserting a close date in the database.
[x] In ./public/activity.php, against each assigned task in the "My Tasks" section there should be a book time agains the task. This should open a dialog box asking for the date and the hours to book for that task.
[x] In ./public/activity.php, for the task tree the status of tasks with children tasks should be determined as follows.  If the task is open with any user the task should be shown as in-progress. If a task has childern determine the status of that task based on the status of the children: if all the childern tasks are complete mark the task as complete,  if all children tasks are open then mark the task as open, otherwise mark the task as in-progress.
[x] In ./public/activity.php, for the task tree, display the hours consumed for that task within parentheses after the task name. If the task has children the hours of the childern should be added to the hours booked on the task.
[x] Move progress section from ./public/artefacts.php to a new page ./public/milestones.php. Format the new page similar to artefacts.php.
[x] In ./public/artefact.php, after the artefact type add a row to show the artefact owner.
[x] In ./public/artefact.php, after the artefact owner add a row to show the a comma separated list of activities linked to the artefact.
[x] In ./public/artefact.php, add a section to visualize the overall progress of the artefact. The format of the graph should follow the gantt chart in ./public/activity.php. The horizontal axis should represent the percentage progress. Each row should display a bar graph the progress of the artefact in each of the linked arctivities, scaled to the ratio of the artefact linked to that activity, followed by the a bar graph showing the overall progress.
[x] In ./public/artefact.php, add a link to ./public/milestones.php. Place an edit button next to the progress bar showing the progress of artefact for that activity which calls milestones.php using the corresponding artefactlinkid.
[x] Security: AuthService::authenticate() now verifies passwords against Employees.passwordsalt using the same salted SHA-256 scheme already implemented in admin/src/utils/auth.cpp (not password_hash()/bcrypt - passwordsalt is actively used by the admin CLI tool, not unused). Fixed the corresponding seed data in database/seed/trial_project.sql, which previously stored a placeholder passwordsalt and would have failed to authenticate under the real scheme.
[x] Security: Add an access check on activity.php/artefact.php/milestones.php confirming the current user is actually part of the activity's project team, not just logged in (anyone can view any activityid/artefactlinkid by editing the hidden form field).
[x] Security: Remove dashboard/config/config.php (unused, duplicates DB credentials already in src/Database.php) and move real DB credentials out of source into an environment variable or git-ignored config.
[x] Security: Add CSRF protection to the POST forms that change data (add/delete/close task, delegate artefact, etc.).
[x] Extract the repeated "session_start + validateSession + redirect if not logged in" boilerplate (duplicated in every page AND in includes/header.php, which runs the same check again) into one shared bootstrap file.
[x] Delete ArtefactService::getMilestoneDetails() (dead code; its name collided with the differently-shaped MilestoneService::getMilestoneDetails() which caused the artefact.php bug fixed on 2026-09-30).
[x] Replace the magic status numbers (0/1/2 for complete/in-progress/not-started) in ActivityService.php and TaskTree.php with named constants.
[x] Standardize the casing of SQL result array keys (mix of Activity_Name and activityname across Services) to reduce "undefined index" mistakes.
[x] Change GanttChart, HoursChart, TaskTree, and ArtefactProgressChart to return HTML strings instead of echoing directly, so they can be tested/reused outside of immediate page output.
[x] Replace the hidden-form-plus-JS navigation (navigateToArtefact, navigateToMilestones) with plain GET links (e.g. activity.php?activityid=5); reserve POST for actions that actually change data.
[x] Delete stray editor files (dashboard/misc/.trial.sql.swo, database/schema/.trial.sql.swp) and add * .sw? to .gitignore.
[x] Clean up or archive the void/ scratch folders under database/, database/schema/, and database/seed/.
[x] Remove the unused Database::query() raw-SQL method (no prepared statement, currently unused, easy to misuse for SQL injection later).
[x] Remove the duplicate "Book Time" button from the dashboard. It is already shown in the header.
[x] The dashboard should comprise of three sections. First should be an option to select the project. Below that the existing activities table. Below that artifacts in the project. 
[x] Create a page tasks.php that shows the details of task assignments and status in a tabular format. This page should be accessible from the activities page on clicking a task in the heirarch on the `Tasks` card.
[x] In the artfacts section of the dashboard should only show the artefact types and the cumulative progress. The artefact table should be moved to the artefacts page. Move artefacts data to its own page accessible by clicking an artefact for the artefacts page.
[x] In the artefacts table of the ./public/artefacts.php, page instead of showing the number of linked activities, show the activity names separated by commas, which link to the corresponding activity page. 
[x] Clicking an artefact in the ./public/artefacts.php page should navigate the user to the artefact_data.php page. There is no need for an intermediate artefact.php page. Also update the navigation from the artefact section in the activity.php page to artefacts.php with a filter for artefactlinkid.
[x] In the artefacts section of the ./public/activity.php page add a column showing a comma separated list of the completed milestones of the artefact.
[x] In the artefacts section of of the ./public/activity.php page add a "..." button at the end with options to delegate artefact and to set milestones.
[x] Completed tasks are shown as "Overdue" in the dashboard. Or the task tree in activity is not showing the correct status.
[x] `actualstart` and `actualfinish` in the all the sql queries such as the one in  getUserActivities() should be calculated from the time the first task is assigned and the time the last task is closed not manually entered.
[x] Remove the Tasks count column and `Details` button from the activities table in the ./public/dashboard.php
[x] Remove the `status` column from My Tasks in ./public/activity.php
[x] Move the `Book time` button to within the `...` option menu in the `My Tasks` section of ./public/activity.php.
[x] Convert the artefacts section of ./public/activity.php into a table with columns `Artefact`, `Type`, `Owner`, `Milestones Completed`, and `Progress`. Status pills are not required.
[x] In the ./public/artefacts.php page move the linked activities into a separate sub-table that can be collapsed and expanded when the artefact is clicked. The table should have columns `Activities Linked`, `Link Ratio`, `Progress`.   
[x] Artefact data ./public/artefact_data.php should be linked through artefactid not artefactlinkid
[x] Create an option to save edits in ./public/ artefact_data.php. If there are changes and the user navigates away show a warning.
[x] Error: Task tree not showing task as complete.
[x] Add an option to add a new artefact type to an activity.
[x] Add an option to add a new artefact
[x] There should be an option to delegate an artefact.
[x] Add planned date, and forecast date to MilestoneToArtefactLink table.
[x] Add a field to table ArtefactDataFields to indicate if a field is editable.
[x] Add a field to table Artefact called artefactdescription.
[x] Add a details button next to the project drop down in the ./public/dashboard.php which navigated to a page project.php and shows details of the selected project such a project manager and team members.
[x] In ./public/dashboard.php, add button next to the `Activities` header to add an activity. Should only be visible to Project Managers.
[x] In ./public/dashboard.php, add a filter button next to the `Activities` header. It should open a modal dialog with the option to filter "My Activities" and "All Activities". "My Activities" should show activity where the user has been assigned task or the user is the activity manager. Change the header accordin g to the option selected.
[x] In ./public/dashboard.php, add a botton to add an artefact next to the `Artefacts` header. This should be visible to the Project Manager only. Remove this option from ./public/activity.php.
[x] In ./public/dashboard.php, add a botton to add an artefact type next to the `Artefacts` header. This should be visible to the Project Manager only.Remove this option from ./public/activity.php.
[x] In ./public/dashboard.php, add a filter button next to the `Artefacts` header. It should open a modal dialog with the option to filter "My Artefacts" and "All Artefacts".Change the header accordin g to the option selected.
[x] In ./public/activity.php, move the `Delegate artefact` and `Set milestone` options from within the `...` button in the artefacts table to individual buttons next to the artefact section header. Add a check box to the artefacts table next to each item so that the user can select multiple artefacts to set milestones for.
[x] In ./pulic/activity.php, in the artefacts section, add an option to link an existing artefact to the activity.Also add an option to remove a linked artefact. These options should only be visible to the project manager and the activity manager.
[x] In ./pulic/activity.php, in the artefacts section, add an option to associate a Milestone with an artefact. This option should only be visible to the project manager.
[x] Remove the `Progress by activity` card from ./public/artefact.php, and add a non-editable card showing the artefact data.
[x] Remove the details button and dropdown sub-table from ./public/artefacts.php. Move the sub table to ./public/artefact.php. Clicking an item in the artefacts.php table should lead to artefact.php.
[x] Clicking on an artefact on ./public/activity.php should take the user to ./public/artefact.php.
[x] In ./public/activity.php, move the schedule and hours charts in to the `overview` card. Move the `edit activity` to the header of this card.
[x] When the user hands back or closes a task from the `My Tasks` section, assigns a task in `task` section or add a note in ./public/activity.php the date used should be the current date but should not be editable.
[x] Move the milestone steps tables from ./public/milestones.php to a sub-table under the `linked activities` section of ./public/artefact/php.
[x] Remove the `linked milestone` and `completed milestone` columns from the linked activities table in ./public/artefact.php 
[x] Remove the `status` column from the milestone steps sub-table in the linked activities section of ./public/artefact.php. Instead add a column `Achieved` that shows the completion date of the step.
[x] In ./public/artefact.php, move the `delegate` button to the header of the overview card. And move the `Data` button to the header of the data card, and rename it as `Edit` with a pencil icon.
[x] Add an edit button in the header of the overview card of ./dashboard/public/artefact.php. This can be used to edit the artefact name and description. This option should be only be available to the artefact owner and the project manager.
[x] Add an edit button to the `linked activities` card header of ./dashboard/public/artefact.php. This can be used to edit the link ratio of all the linked activities.This option should be only be available to the artefact owner and the project manager.
[x] Add a `link activity` button to the header of the `linked activities` card of ./dashboard/public/artefact.php. This can be used to link an activity to the current artefact.This option should be only be available to the artefact owner and the project manager.
[x] Add a `delegate` button to the overview card of ./dashboard/public/activity.php. This can be used to change the activity manager. This option should only be available to the project manager.
[x] In the overview card of ./dashboard/public/artefact.php, add a chart showing the planned, forecast and actual progress of the milestone based on the planned, forecast and actual completion of the milestone steps of associated activities.
[x] In the `My tasks` section of ./dashboard/public/activity.php, move the options from the `Actions` column to individual buttons in the header of the section.
[x] Remove the `Book time` option from the `My tasks` section of ./dashboard/public/activity.php.
[x] In the `My tasks` section of ./dashboard/public/activity.php, use the same checkbox interface used in the `artefacts` section. Update the `Close task` and `Reassign` options to handle the selection of multiple tasks. The checkbox should only be activiated when it is clicked. Clicking anywhere else on the task row should take the user to the tasks.php for that task.
[x] In ./dashboard/public/activity.php, the schedule gantt chart does not scale properly when the actual duration is very large. The chart seems to go out of the screen. 
[x] In ./dashboard/public/project.php, the overview and team member cards should span the full width of the page.
[x] In ./dashboard/public/project.php, add a card that shows the milestones in a project. Add options to add, edit or remove a milestone. These options should only be available to the project manager.
[x] In ./dashboard/public/project.php, in the header for the team member card, add an option to add a team member
[x] In ./dashboard/public/project.php, in the header for the overview card, add an option to edit the project details.
[x] In ./dashboard/public/book_time.php, the new `entry card` should be moved into a modal dialog that is accessed through a `Book Time` option in the header for `My current task assignments`.
[x] In ./dashboard/public/tasks.php, add an option in the `Task details` card to edit task details. This option should be available to the Manager, Lead, and Activity manager.
[x] In ./dashboard/public/tasks.php, add an option in the Assignements card header to close an open task. This option should be available if the user has an open assignement in the task. If the user is an Activity Manager the option should be to close the task.
[x] In ./dashboard/public/tasks.php, add an option in the Assignements card header to reassign an open task. This option should be available if the user has an open assignement in the task, the Activity Manager, Lead and Manager.
[x] Add an option to delete task in the `task details` section of ./dashboard/public/tasks.php. Remove this option from the tasks section of ./dashboard/public/activity.php.
[x] Add an option to assign a task in the `assignments` section of ./dashboard/public/tasks.php. This should be enabled when the task is closed and should be visible to the activity manager, lead and manager. 
[x] Add an option to reassign an open task in the `tasks` section of ./dashboard/public/activity.php. This option should only be visible to the activity manager, lead and manager.
[x] In ./dashboard/public/artefact_data.php, the bread crumbs should point back to the artefact in artefact.php.
[x] In ./dashboard/public/artefact_data.php, the save and discard changes options should only be visible to the Artefact Owner, Lead and Manager.
[x] Move the option to `add artefact` from ./dashboard/public/dashboard.php to ./dashboard/public/artefacts.php.
[x] In ./dashboard/public/artefacts.php, add a column of check boxes so that multiple artefacts can be selected at once.
[x] In ./dashboard/public/artefacts.php, add an option to delete an artefact. This options should only be available to the Manager
[x] In ./dashboard/public/artefacts.php, add an option to delegate an artefact. This option should only be available to the Manager and Lead.
