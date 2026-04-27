# Osoem - A Database for Engineering Project Management

Engineering projects are often large and complex endeavors that generate lots of data. This data can include the dates document are issued, time spent performing activities, quality compliance checks, commodities etc. Managing this data is crucial in effectively managing the project. 

However, there are few database systems designed to cater specifically to engineering projects. In a typical engineering project, disparate data management systems are combined to manage the data. For example, there may be a payroll processing system to log time, a project management tool like P6 to track activities, and a document control package to manage deliverables. The issue with this methodology is that it does not capture the relationships that these databases have with each other, which can lead to inefficiencies such as data duplication or inconsistent data. 

Engineering project data can broadly be divided into four categories:
- **People** People with different roles come together to execute the project. 
- **Activities** All engineering projects can be divided into smaller and manageable parts called activities.
- **Time** Time is often the basis of progress measurements in engineering projects.
- **Artefacts** Here we use Artefacts as an umbrella term to represent all the different types of deliverables in engineering projects.

The project comprises of the following parts:
1. The database
2. The adminstration tool
3. The front end app

## The Database

The complete database schema is provided in file:
'./database/schema/schema.sql'

A description of the database follows.

### Managing People

Project management is all about coordinating the activities of a group of people to achieve a common objective. In our database we require a table to store the information of the people in the team. For this purpose we create an `Employees` table.

### Managing Activities 

Dividing a project into activities helps to simplify the scope of the work handled by the team. When determining the activities of a project, it is important to define the scope of each activity. If the scope is too broad it may become too complex to handle, and if the scope is too granular, the number of activities in a project would become difficult to manage. To organize project work, activities are typically grouped together into a hierarchy. This is commonly called a Work Breakdown Structure. We sort activities into categories and sub-categories. Each activity will have a planned start date and end date, and the budgeted hours required for execution. The forecasts and actuals can then be used to measure the state of the activity. An activity owner would be delegated for the planning and execution of the activity. Activities are further divided into tasks. While an activity may be planned for at the start of a project, it would be better if tasks are planned during the execution of the project. With this structure we can ensure flexibility in the execution and account for uncertainties. Tasks have a hierarchical structure and can be broken up further as required. It is the activity owner's responsibility to organize the tasks in their activity and to decide who to assign these tasks to. Once a task is assigned a user can book time against that task.

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

To represent this in our database, first we create the `Projects` tables to save the project information.

Next we create the `ActivityCategories` and `ActivitySubcategories` tables to define the work breakdown structure.

Next step is to define a `Activities` table to store the activity information. 

We want the user to be able to record unstructured information in the form of notes. For this we create a separate table called `ActivityNotes` that stores the notes, date, and identifies the user making the comment.

To store the information of the tasks within an activity we create the `ActivityTasks` table.

We use the `parenttaskid` field to define a hierarchical structure for the tasks. As the name suggests, this will point to the parent task. If the value of this field is `null` that task is on top of the hierarchy and does not have a parent task. Tasks can be assigned to users by the activity manager. To capture this information we create a table to store assignment details.
Here we save the date the task is assigned and the date the task is completed. 

### Managing Time

Time is probably the most important resource to manage in an engineering project. It is normally measured in hours required to complete a task. To facilitate this we create a `Hours` table to save the hours booked by users on tasks.

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

We define the `ArtefactTypes` table to store the types of artefacts in a project. 

The next we create the `Artefacts` table that holds the data of each artefact. 

We also need a `ArtefactDataFields` table to define the data fields that each artefact type would store.  The `valuetype` field is takes an integer value to determine the type of value that the field can store this can be either 0 for text, or 1 for numbers. The fields `maximumlength`, `maximumvalue` and `minimumvalue` are used to perform basic validations. The number of characters in a text field is stored in `maximumlength`. For numbers, we can decide what is the minimum and maximum allowable values it can accept.

To store the values of the artefact data fields we create another table `ArtefactData`. The `value` field stores the the value of the field. The `artefactid` field links the data to the artefact and the `artefactdatafieldid` identifies the field to which the value belongs. Note that we are arbitrarily limiting the _value_ to be of 2000 characters in length. In the `ArtefactDataFields` table we let the user define the maximum length of text fields. Thus a user is able to define any length for text field up to a limit of 2000 characters. Also note that we are storing numbers also as text values; we use `valuetype` from `ArtefactDataFields` table to determine if the value stored is text or a number. 

We can now link the artefact to an activity using the `ArtefactToActivityLink` table. To allow the same artefact to be shared between multiple activities we define the proportion of the artefact to associate to a activity in `ratio`. The value of _ratio_ should be between zero and one, and the sum of all the ratios in all links for a particular artefact should not exceed one. For example, if a drawing is prepared by combining two activities, we can create two links -- each associated with a 50% of the drawing. 

To measure the progress of artefacts we need to define the types of milestones that we will use in a project in the `Milestones` table. Each milestone will have multiple steps associated with it. This is defined in the `MilestoneSteps` table.  The `progressratio` field store the ratio of progress that should be assigned to the artefact on completion of the step.

Finally, we link the milestone to the artefact, and through the artefact it is linked to an activity through the `MilestoneToArtefactLink` table.

## The Administration Tool

The admin tool is a command line application that is used to navigate the database and made modifications to it. The tool and the associated files are in the following folder:
'./admin/'

For further details of the osoem admin tool see:
'./admin/CLAUDE.md'

## The Front End App

The front end application and associated files are in the following folder:
'./app/'
