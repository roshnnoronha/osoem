[x] In ./dashboard/public/artefacts.php, move the artefact type filter into its own modal dialog accessible through a `filter` option.
[x] Remove `reassign task` from the `my tasks` section of ./dashboard/public/activity.php
[x] Remove the `delegate artefact` option from the artefact section of ./dashboard/public/activity.php
[x] The add type modal in ./dashboard/public/dashboard.php should have options to add data fields to the artefact type. 
[x] Move the delete option from ./dashboard/public/artefacts.php to the overview card of ./dashboard/public/artefact.php
[x] Create an option to remove a link in the `linked activities` card of ./dashboard/public/artefact.php

[ ] Close task and complete task should be separate options?

[ ] Add real automated tests (e.g. PHPUnit for the Services classes) - misc/test_plan.txt is a plan document only, not actual test coverage.
[ ] Bug: ActivityService::isProjectManager() selects Projects.projectmanagerid, but the Projects table (database/schema/schema.sql) has no such column - this will throw a SQL error whenever it runs (e.g. gating the "Edit Activity" button/form in activity.php). Either add the column or derive the project manager from ProjectTeamMembers.role instead.
[ ] Login as an lead, activity manager, artefact owner, member, to check UI.




