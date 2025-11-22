# A Database for Engineering Project Management

Engineering projects are often large and complex endeavors that generate lots of data. This data can include the dates document are issued, time spent performing activities, quality compliance checks, commodities etc. Managing this data is crucial in effectively managing the project. 

However, there are few database systems designed to cater specifically to engineering projects. In a typical engineering project, disparate data management systems are combined to manage the data. For example, there may be a payroll processing system to log time, a project management tool like P6 to track activities, and a document control package to manage deliverables. The issue with this methodology is that it does not capture the relationships that these databases have with each other, which can lead to inefficiencies such as data duplication or inconsistent data. 

## Creating the Database

Engineering project data can broadly be divided into four categories:
- **People** People with different roles come together to execute the project. 
- **Activities** All engineering projects can be divided into smaller and manageable parts called activities.
- **Time** Time is often the basis of progress measurements in engineering projects.
- **Artefacts** Here we use Artefacts as an umbrella term to represent all the different types of deliverables in engineering projects.
We will deal with each of these in detail in this section, developing a database schema that models the relationships between them.

### Managing People

Project management is all about coordinating the activities of a group of people to achieve a common objective. In our database we require a table to store the information of the people in the team. For this purpose we create an `Employees` table.

```sql
CREATE TABLE Employees(
    employeeid INT NOT NULL AUTO_INCREMENT,
    firstname VARCHAR(255) NOT NULL,
    lastname VARCHAR(255) NOT NULL,
    password VARCHAR(255) NOT NULL,
    email VARCHAR(255) NOT NULL,
    PRIMARY KEY (employeeid)
);
```

### Managing Activities 

Dividing a project into activities helps to simplify the scope of the work handled by the team. When determining the activities of a project, it is important to define the scope of each activity. If the scope is too broad it may become too complex to handle, and if the scope is too granular, the number of activities in a project would become difficult to manage. 

To organize project work, activities are typically grouped together into a hierarchy. This is commonly called a Work Breakdown Structure. We sort activities into categories and sub-categories. 

Each activity will have a planned start date and end date, and the budgeted hours required for execution. The forecasts and actuals can then be used to measure the state of the activity. An activity owner would be delegated for the planning and execution of the activity.

Activities are further divided into tasks. While an activity may be planned for at the start of a project, it would be better if tasks are planned during the execution of the project. With this structure we can ensure flexibility in the execution and account for uncertainties. Tasks have a hierarchical structure and can be broken up further as required. It is the activity owner's responsibility to organize the tasks in their activity and to decide who to assign these tasks to. Once a task is assigned a user can book time against that task.

Let us say, for example, that we have to engineer a residential building. The project can be first divided into activities.

```
Residential Building Project
    ├─ Prepare architectural drawings
    ├─ Prepare structural drawings
    ├─ Prepare plumbing drawings
    └─ Prepare electrical wiring drawings
```
The activity ***Prepare structural drawings*** can be further broken up into tasks.

```
Prepare structural drawings 
    ├─ 1.0 Review architectural drawings 
    ├─ 2.0 3D model preparation 
    │   ├─ 2.1 Prepare a preliminary 3D model
    │   ├─ 2.2 Prepare final 3D model 
    │   └─ 2.3 Perform clash check 
    ├─ 3.0 Analysis and design
    │   ├─ 3.1 Define the structure geometry 
    │   ├─ 3.2 Apply the loads on the analysis model
    │   ├─ 3.3 Define the design parameters for the model 
    │   └─ 3.4 Run the analysis and optimize the design
    ├─ 4.0 Drawing prepartion
    │   ├─ 4.1 Prepare input for drawing drafting 
    │   ├─ 4.2 Drawing preparation
    │   ├─ 4.3 Drawing review
    │   └─ 4.4 Drawing comment incorporation
    ├─ 5.0 Calculation report compilation
    │   ├─ 5.1 Calculation preparation
    │   ├─ 5.2 Calculation review
    │   └─ 5.3 Calculation comment incorporation
    ├─ 6.0 Interdisciplinary checks
    │   ├─ 6.1 Perform disciplinary checks
    │   └─ 6.2 Incorporate comments
    └─ 7.0 Document Approval
```

To represent this in our database, first we create the tables to save the project information.
```sql
--Tables for projects
CREATE TABLE Projects(
    projectid INT NOT NULL AUTO_INCREMENT,
    projectmanagerid INT NOT NULL,
    projectno VARCHAR(255) NOT NULL UNIQUE,
    projectname VARCHAR(255) NOT NULL,
    PRIMARY KEY (projectid),
    FOREIGN KEY (projectmanagerid) REFERENCES Employees (employeeid)
);
```

Next we create the tables to define the work breakdown structure.

```sql
--Table for activity categories
CREATE TABLE ActivityCategories(
    categoryid INT NOT NULL AUTO_INCREMENT,
    projectid INT NOT NULL,
    categoryname VARCHAR(255) NOT NULL,
    PRIMARY KEY (categoryid),
    FOREIGN KEY (projectid) REFERENCES Projects (projectid)
);
--Table for activity sub-categories
CREATE TABLE ActivitySubcategories(
    subcategoryid INT NOT NULL AUTO_INCREMENT,
    categoryid INT NOT NULL,
    subcategoryname VARCHAR(255),
    PRIMARY KEY (subcategoryid),
    FOREIGN KEY (categoryid) REFERENCES ActivityCategories (categoryid)
);
```
Next step is to define a table to store the activity information. 

```sql
--Table for activities
CREATE TABLE Activities(
    activityid INT AUTO_INCREMENT NOT NULL,
    subcategoryid INT NOT NULL,
    activityname VARCHAR(255) NOT NULL,
    activitydescription VARCHAR(1000),
    activitymanagerid INT NOT NULL,
    plannedstart DATE,
    plannedfinish DATE,
    forecaststart DATE,
    forecastfinish DATE,
    actualstart DATE,
    actualfinish DATE,
    plannedhours DECIMAL(10,2),
    forecasthours DECIMAL(10,2),
    PRIMARY KEY (activityid),
    FOREIGN KEY (subcategoryid) REFERENCES ActivitySubcategories (subcategoryid),
    FOREIGN KEY (activitymanagerid) REFERENCES Employees (employeeid)
);
```

We want the user to be able to record unstructured information in the form of notes. For this we create a separate table called `ActivityNotes` that stores the notes, date, and identifies the user making the comment.

```sql
--Table to record notes and comments on activities
CREATE TABLE ActivityNotes(
    noteid INT AUTO_INCREMENT NOT NULL,
    activityid INT NOT NULL,
    userid INT NOT NULL,
    note VARCHAR(2000) NOT NULL,
    notedate DATE NOT NULL,
    PRIMARY KEY (noteid),
    FOREIGN KEY (activityid) REFERENCES Activities (activityid),
    FOREIGN KEY (userid) REFERENCES Employees (employeeid)
);
```

To store the information of the tasks within an activity we create the `ActivityTasks` table.

```sql
--Table for tasks
CREATE TABLE ActivityTasks(
    taskid INT NOT NULL AUTO_INCREMENT,
    activityid INT NOT NULL,
    taskname VARCHAR(255) NOT NULL,
    parenttaskid INT,
    PRIMARY KEY (taskid),
    FOREIGN KEY (activityid) REFERENCES Activities(activityid)
);
```

We use the `parenttaskid` field to define a hierarchical structure for the tasks. As the name suggests, this will point to the parent task. If the value of this field is `null` that task is on top of the hierarchy and does not have a parent task. 

Tasks can be assigned to users by the activity manager. To capture this information we create a table to store assignment details.

```sql
--Table for task assignments
CREATE TABLE ActivityTaskAssignments(
    assignmentid INT NOT NULL AUTO_INCREMENT,
    taskid INT NOT NULL,
    userid INT NOT NULL,
    assigneddate DATE,
    closedate DATE,
    PRIMARY KEY (assignmentid),
    FOREIGN KEY (taskid) REFERENCES ActivityTasks (taskid),
    FOREIGN KEY (userid) REFERENCES Employees (employeeid)
);
```
Here we save the date the task is assigned and the date the task is completed. 

### Managing Time

Time is probably the most important resource to manage in an engineering project. It is normally measured in hours required to complete a task. To facilitate this we create a table to save the hours booked by users on tasks.

```sql
CREATE TABLE Hours(
    bookingid INT NOT NULL AUTO_INCREMENT,
    taskid INT NOT NULL,
    userid INT NOT NULL,
    bookeddate DATE NOT NULL DEFAULT CURRENT_TIMESTAMP,
    hours INT NOT NULL,
    PRIMARY KEY (bookingid),
    FOREIGN KEY (taskid) REFERENCES ActivityTasks(taskid),
    FOREIGN KEY (userid) REFERENCES Employees(employeeid) 
);
```

### Managing Artefacts

An artefact is an abstraction used to represent the various outputs and inputs that goes into the completion of engineering activities. This enables us to create a common interface to link deliverables to activities. 

Some examples of artefacts are -- engineering documents like drawings and calculations, commodities like concrete, engineering inputs, vendor documents, review comments etc.

Once we define the types of artefacts in a project, we can describe the data we want these artefacts store. This data is stored in fields. Since artefacts can represent any deliverable associated with engineering activities these fields must be user defined and customizable. For example, for an artefact representing a drawing we may want to track the following information.

```
+---------------------+
| Field name          |    
+---------------------+
| Drawing number      |         
| Drawing title       |        
| Revision            |        
| Revision purpose    |        
| Prepared by         |        
| Checked by          |        
| Approved by         |        
+---------------------+
```

To measure the progress of artefacts we define milestones. A milestone is a predefined step in the development of the deliverable that we want to assign a portion of the progress to -- defined as a ratio between zero and one. Thus when milestones are marked as complete we can calculate the progress of the artefact using the corresponding ratio. For example, a drawing can have the following milestones. 

```
+-----------------------------+----------------+
| Milestone Step              | Progress Ratio | 
+-----------------------------+----------------+ 
| Discipline check            | 0.50           | 
| Inter-disciplinary check    | 0.75           | 
| Issue for Construction      | 1.00           | 
+-----------------------------+----------------+ 
```

First, we shall define the table to store the types of artefacts in a project. 

```sql
CREATE TABLE ArtefactTypes(
    artefacttypeid INT AUTO_INCREMENT NOT NULL,
    projectid INT NOT NULL,
    artefactname VARCHAR(255) NOT NULL,
    artefactdescription VARCHAR(1000),
    PRIMARY KEY (artefacttypeid),
    FOREIGN KEY (projectid) REFERENCES Projects (projectid)
);
```

The next we create the table that holds the data of each artefact. 

```sql
CREATE TABLE Artefacts(
    artefactid INT AUTO_INCREMENT NOT NULL,
    artefacttypeid INT NOT NULL,
    artefactownerid INT,
    artefacttitle VARCHAR(255) NOT NULL,
    PRIMARY KEY (artefactid),
    FOREIGN KEY (artefacttypeid) REFERENCES ArtefactTypes (artefacttypeid),
    FOREIGN KEY (artefactownerid) REFERENCES Employees (employeeid) 
);
```

We also need a table to define the data fields that each artefact type would store.

```sql
CREATE TABLE ArtefactDataFields(
    artefactdatafieldid INT AUTO_INCREMENT NOT NULL,
    artefacttypeid INT NOT NULL,
    fieldtitle VARCHAR(255) NOT NULL,
    valuetype INT NOT NULL,
    maximumlength INT,
    maximumvalue INT,
    minimumvalue INT,
    PRIMARY KEY (artefactdatafieldid),
    FOREIGN KEY (artefacttypeid) REFERENCES ArtefactTypes (artefacttypeid)
);
```

The `valuetype` field is takes an integer value to determine the type of value that the field can store this can be either 0 for text, or 1 for numbers. The fields `maximumlength`, `maximumvalue` and `minimumvalue` are used to perform basic validations. The number of characters in a text field is stored in `maximumlength`. For numbers, we can decide what is the minimum and maximum allowable values it can accept.

To store the values of the artefact data fields we create another table. 

```sql
CREATE TABLE ArtefactData(
    artefactdataid INT AUTO_INCREMENT NOT NULL,
    artefactid INT NOT NULL,
    artefactdatafieldid INT NOT NULL,
    value VARCHAR(2000),
    PRIMARY KEY (artefactdataid),
    FOREIGN KEY (artefactid) REFERENCES Artefacts (artefactid),
    FOREIGN KEY (artefactdatafieldid) REFERENCES ArtefactDataFields (artefactdatafieldid) 
);
``` 

The `value` field stores the the value of the field. The `artefactid` field links the data to the artefact and the `artefactdatafieldid` identifies the field to which the value belongs. Note that we are arbitrarily limiting the _value_ to be of 2000 characters in length. In the `ArtefactDataFields` table we let the user define the maximum length of text fields. Thus a user is able to define any length for text field up to a limit of 2000 characters. Also note that we are storing numbers also as text values; we use `valuetype` from `ArtefactDataFields` table to determine if the value stored is text or a number. 

We can now link the artefact to an activity.

```sql
CREATE TABLE ArtefactToActivityLink(
    artefactlinkid INT AUTO_INCREMENT NOT NULL,
    activityid INT NOT NULL,
    artefactid INT NOT NULL,
    ratio DECIMAL(3,2),
    PRIMARY KEY (artefactlinkid),
    FOREIGN KEY (activityid) REFERENCES Activities (activityid),
    FOREIGN KEY (artefactid) REFERENCES Artefacts (artefactid)
);
```

To allow the same artefact to be shared between multiple activities we define the proportion of the artefact to associate to a activity in `ratio`. The value of _ratio_ should be between zero and one, and the sum of all the ratios in all links for a particular artefact should not exceed one. For example, if a drawing is prepared by combining two activities, we can create two links -- each associated with a 50% of the drawing. 

To measure the progress of artefacts we need to define the types of milestones that we will use in a project.

```sql
CREATE TABLE Milestones(
    milestoneid INT AUTO_INCREMENT NOT NULL,
    projectid INT NOT NULL,
    milestonename VARCHAR(255) NOT NULL,
    PRIMARY KEY (milestoneid),
    FOREIGN KEY (projectid) REFERENCES Projects (projectid)
);
```

	Each milestone will have multiple steps associated with it. This is defined in the `MilestoneSteps` table.

```sql
CREATE TABLE MilestonesSteps(
    milestonestepid INT AUTO_INCREMENT NOT NULL, 
    milestoneid INT NOT NULL,
    milestonestepname VARCHAR(255) NOT NULL,
    progressratio DECIMAL(3,2) DEFAULT 0.00,
    PRIMARY KEY (milestonestepid),
    FOREIGN KEY (milestoneid) REFERENCES Milestones (milestoneid) 
);
```

The `progressratio` field store the ratio of progress that should be assigned to the artefact on completion of the step.

Finally, we link the milestone to the artefact, and through the artefact it is linked to an activity.

```sql
CREATE TABLE MilestoneToArtefactLink(
    milestonelinkid INT AUTO_INCREMENT NOT NULL,
    milestonestepid INT NOT NULL,
    artefactlinkid INT NOT NULL,
    completiondate DATE,
    completedby INT,
    approveddate DATE,
    approvedby INT,
    PRIMARY KEY (milestonelinkid),
    FOREIGN KEY (milestonestepid) REFERENCES MilestoneSteps (milestonestepid),
    FOREIGN KEY (artefactlinkid) REFERENCES ArtefactToActivityLink (artefactlinkid)
); 
```

