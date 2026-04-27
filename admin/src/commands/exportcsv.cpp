#include "exportcsv.h"

#include "../exceptions/exceptions.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace commands {

// Helper function to escape CSV fields
std::string escapeCSV(const std::string& field) {
    if (field.find(',') != std::string::npos ||
        field.find('"') != std::string::npos ||
        field.find('\n') != std::string::npos) {
        std::string escaped = "\"";
        for (char c : field) {
            if (c == '"') {
                escaped += "\"\"";
            } else {
                escaped += c;
            }
        }
        escaped += "\"";
        return escaped;
    }
    return field;
}

void exportcsv (Database& db, Path& pth, std::string filename) {
    if (filename.empty()) {
        throw FileIOError("No filename specified.");
    }

    // Open file for writing
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw FileIOError("Could not open file '" + filename + "' for writing");
    }

    std::cout << "Exporting data to file: " << filename << std::endl;

    try {
        if (pth.current_type() == Path::ROOT){
            throw NavigationError("Cannot export from root. Navigate to a specific folder.");
        } else if (pth.current_type() == Path::ORGANIZATION_FOLDER){
            throw NavigationError("Cannot export here. Navigate to Departments.");
        } else if (pth.current_type() == Path::DEPARTMENTS_FOLDER){
            // Export all departments
            std::string query = "SELECT departmentname FROM Departments ORDER BY departmentid";
            auto res = db.executeQuery(query);

            file << "departmentname" << std::endl;

            int count = 0;
            while (res->next()) {
                file << escapeCSV(res->getString("departmentname")) << std::endl;
                count++;
            }
            std::cout << "Exported " << count << " departments" << std::endl;

        } else if (pth.current_type() == Path::DEPARTMENT){
            // Export employees in the current department
            std::string query = "SELECT firstname, lastname, email FROM Employees WHERE departmentid = ? ORDER BY employeeid";
            auto stmt = db.prepareStatement(query);
            stmt->setInt(1, pth.current_id());
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            file << "firstname,lastname,email" << std::endl;

            int count = 0;
            while (res->next()) {
                file << escapeCSV(res->getString("firstname")) << ","
                     << escapeCSV(res->getString("lastname")) << ","
                     << escapeCSV(res->getString("email")) << std::endl;
                count++;
            }
            std::cout << "Exported " << count << " employees" << std::endl;

        } else if (pth.current_type() == Path::PROJECTS_FOLDER){
            // Export projects
            std::string query = "SELECT projectname, projectno FROM Projects ORDER BY projectid";
            auto res = db.executeQuery(query);

            // Write header
            file << "projectname,projectno" << std::endl;

            // Write data
            int count = 0;
            while (res->next()) {
                file << escapeCSV(res->getString("projectname")) << ","
                     << escapeCSV(res->getString("projectno")) << std::endl;
                count++;
            }
            std::cout << "Exported " << count << " projects" << std::endl;

        } else if (pth.current_type() == Path::PROJECT){
            throw NavigationError("Cannot export here. Navigate to Activities, Artefact Types, Milestones, or Team folder.");
        } else if (pth.current_type() == Path::TEAM_FOLDER){
            // Export team members for the current project
            std::string query = R"(
                SELECT e.firstname, e.lastname, e.email, ptm.role
                FROM ProjectTeamMembers ptm
                JOIN Employees e ON ptm.employeeid = e.employeeid
                WHERE ptm.projectid = ?
                ORDER BY ptm.teammemberid
            )";
            auto stmt = db.prepareStatement(query);
            stmt->setInt(1, pth.current_id());
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            file << "firstname,lastname,email,role" << std::endl;

            int count = 0;
            while (res->next()) {
                file << escapeCSV(res->getString("firstname")) << ","
                     << escapeCSV(res->getString("lastname")) << ","
                     << escapeCSV(res->getString("email")) << ","
                     << escapeCSV(res->getString("role")) << std::endl;
                count++;
            }
            std::cout << "Exported " << count << " team members" << std::endl;

        } else if (pth.current_type() == Path::PROJECT_TEAM_MEMBER){
            throw NavigationError("Cannot export from an individual team member. Navigate to the Team folder.");

        } else if (pth.current_type() == Path::ACTIVITIES_FOLDER){
            // Export categories for the current project
            std::string query = "SELECT categoryname FROM ActivityCategories WHERE projectid = ? ORDER BY categoryid";
            auto stmt = db.prepareStatement(query);
            stmt->setInt(1, pth.current_id());
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            // Write header
            file << "categoryname" << std::endl;

            // Write data
            int count = 0;
            while (res->next()) {
                file << escapeCSV(res->getString("categoryname")) << std::endl;
                count++;
            }
            std::cout << "Exported " << count << " categories" << std::endl;

        } else if (pth.current_type() == Path::CATEGORY){
            // Export subcategories for the current category
            std::string query = "SELECT subcategoryname FROM ActivitySubcategories WHERE categoryid = ? ORDER BY subcategoryid";
            auto stmt = db.prepareStatement(query);
            stmt->setInt(1, pth.current_id());
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            // Write header
            file << "subcategoryname" << std::endl;

            // Write data
            int count = 0;
            while (res->next()) {
                file << escapeCSV(res->getString("subcategoryname")) << std::endl;
                count++;
            }
            std::cout << "Exported " << count << " subcategories" << std::endl;

        } else if (pth.current_type() == Path::SUBCATEGORY){
            // Export activities for the current subcategory
            std::string query = R"(
                SELECT activityname, activitydescription, activitymanagerid,
                       plannedstart, plannedfinish, forecaststart, forecastfinish,
                       actualstart, actualfinish, plannedhours, forecasthours
                FROM Activities
                WHERE subcategoryid = ?
                ORDER BY activityid
            )";
            auto stmt = db.prepareStatement(query);
            stmt->setInt(1, pth.current_id());
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            // Write header
            file << "activityname,activitydescription,activitymanagerid,plannedstart,plannedfinish,"
                 << "forecaststart,forecastfinish,actualstart,actualfinish,plannedhours,forecasthours" << std::endl;

            // Write data
            int count = 0;
            while (res->next()) {
                file << escapeCSV(res->getString("activityname")) << ","
                     << escapeCSV(res->getString("activitydescription")) << ","
                     << res->getInt("activitymanagerid") << ",";

                // Handle nullable date fields
                if (!res->isNull("plannedstart")) {
                    file << res->getString("plannedstart");
                }
                file << ",";
                if (!res->isNull("plannedfinish")) {
                    file << res->getString("plannedfinish");
                }
                file << ",";
                if (!res->isNull("forecaststart")) {
                    file << res->getString("forecaststart");
                }
                file << ",";
                if (!res->isNull("forecastfinish")) {
                    file << res->getString("forecastfinish");
                }
                file << ",";
                if (!res->isNull("actualstart")) {
                    file << res->getString("actualstart");
                }
                file << ",";
                if (!res->isNull("actualfinish")) {
                    file << res->getString("actualfinish");
                }
                file << ",";
                if (!res->isNull("plannedhours")) {
                    file << res->getDouble("plannedhours");
                }
                file << ",";
                if (!res->isNull("forecasthours")) {
                    file << res->getDouble("forecasthours");
                }
                file << std::endl;
                count++;
            }
            std::cout << "Exported " << count << " activities" << std::endl;

        } else if (pth.current_type() == Path::ACTIVITY){
            throw NavigationError("Cannot export here. Navigate to Tasks or Notes folder.");
        } else if (pth.current_type() == Path::TASKS_FOLDER){
            // Export tasks for the current activity
            std::string query = "SELECT taskname, parenttaskid FROM ActivityTasks WHERE activityid = ? ORDER BY parenttaskid ASC, taskid";
            auto stmt = db.prepareStatement(query);
            stmt->setInt(1, pth.current_id());
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            // Write header
            file << "taskname,parenttaskid" << std::endl;

            // Write data
            int count = 0;
            while (res->next()) {
                file << escapeCSV(res->getString("taskname")) << ",";
                if (!res->isNull("parenttaskid")) {
                    file << res->getInt("parenttaskid");
                }
                file << std::endl;
                count++;
            }
            std::cout << "Exported " << count << " tasks" << std::endl;

        } else if (pth.current_type() == Path::TASK || pth.current_type() == Path::SUBTASK){
            // Export subtasks for the current task
            auto taskStmt = db.prepareStatement("SELECT activityid FROM ActivityTasks WHERE taskid = ?");
            taskStmt->setInt(1, pth.current_id());
            auto taskRes = std::unique_ptr<sql::ResultSet>(taskStmt->executeQuery());

            if (taskRes->next()) {
                int actId = taskRes->getInt("activityid");
                std::string query = "SELECT taskname, parenttaskid FROM ActivityTasks WHERE activityid = ? AND parenttaskid = ? ORDER BY taskid";
                auto stmt = db.prepareStatement(query);
                stmt->setInt(1, actId);
                stmt->setInt(2, pth.current_id());
                auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

                // Write header
                file << "taskname,parenttaskid" << std::endl;

                // Write data
                int count = 0;
                while (res->next()) {
                    file << escapeCSV(res->getString("taskname")) << ","
                         << res->getInt("parenttaskid") << std::endl;
                    count++;
                }
                std::cout << "Exported " << count << " subtasks" << std::endl;
            } else {
                throw EntityNotFoundError("Activity", "for current task");
            }

        } else if (pth.current_type() == Path::NOTES_FOLDER) {
            // Export notes for the current activity
            std::string query = R"(
                SELECT userid, note, notedate
                FROM ActivityNotes
                WHERE activityid = ?
                ORDER BY noteid
            )";
            auto stmt = db.prepareStatement(query);
            stmt->setInt(1, pth.current_id());
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            // Write header
            file << "userid,note,notedate" << std::endl;

            // Write data
            int count = 0;
            while (res->next()) {
                file << res->getInt("userid") << ","
                     << escapeCSV(res->getString("note")) << ","
                     << res->getString("notedate") << std::endl;
                count++;
            }
            std::cout << "Exported " << count << " notes" << std::endl;

        } else if (pth.current_type() == Path::ARTEFACT_TYPES_FOLDER) {
            // Export artefact types for the current project
            std::string query = R"(
                SELECT artefactname, artefactdescription
                FROM ArtefactTypes
                WHERE projectid = ?
                ORDER BY artefacttypeid
            )";
            auto stmt = db.prepareStatement(query);
            stmt->setInt(1, pth.current_id());
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            // Write header
            file << "artefactname,artefactdescription" << std::endl;

            // Write data
            int count = 0;
            while (res->next()) {
                file << escapeCSV(res->getString("artefactname")) << ","
                     << escapeCSV(res->getString("artefactdescription")) << std::endl;
                count++;
            }
            std::cout << "Exported " << count << " artefact types" << std::endl;

        } else if (pth.current_type() == Path::ARTEFACT_TYPE){
            throw NavigationError("Cannot export here. Navigate to Artefacts or Fields folder.");
        } else if (pth.current_type() == Path::ARTEFACTS_FOLDER){
            // Export artefacts for the current artefact type
            std::string query = R"(
                SELECT artefacttitle, artefactownerid
                FROM Artefacts
                WHERE artefacttypeid = ?
                ORDER BY artefactid
            )";
            auto stmt = db.prepareStatement(query);
            stmt->setInt(1, pth.current_id());
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            // Write header
            file << "artefacttitle,artefactownerid" << std::endl;

            // Write data
            int count = 0;
            while (res->next()) {
                file << escapeCSV(res->getString("artefacttitle")) << ",";
                if (!res->isNull("artefactownerid")) {
                    file << res->getInt("artefactownerid");
                }
                file << std::endl;
                count++;
            }
            std::cout << "Exported " << count << " artefacts" << std::endl;

        } else if (pth.current_type() == Path::FIELDS_FOLDER){
            // Export fields for the current artefact type
            std::string query = R"(
                SELECT fieldtitle, valuetype, maximumlength, maximumvalue, minimumvalue
                FROM ArtefactDataFields
                WHERE artefacttypeid = ?
                ORDER BY artefactdatafieldid
            )";
            auto stmt = db.prepareStatement(query);
            stmt->setInt(1, pth.current_id());
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            // Write header
            file << "fieldtitle,valuetype,maximumlength,maximumvalue,minimumvalue" << std::endl;

            // Write data
            int count = 0;
            while (res->next()) {
                file << escapeCSV(res->getString("fieldtitle")) << ","
                     << res->getInt("valuetype") << ",";
                if (!res->isNull("maximumlength")) {
                    file << res->getInt("maximumlength");
                }
                file << ",";
                if (!res->isNull("maximumvalue")) {
                    file << res->getInt("maximumvalue");
                }
                file << ",";
                if (!res->isNull("minimumvalue")) {
                    file << res->getInt("minimumvalue");
                }
                file << std::endl;
                count++;
            }
            std::cout << "Exported " << count << " fields" << std::endl;

        } else if (pth.current_type() == Path::DATA_FOLDER){
            // Export artefact field data for the current artefact
            std::string query = R"(
                SELECT adf.fieldtitle, ad.value
                FROM ArtefactData ad
                JOIN ArtefactDataFields adf ON ad.artefactdatafieldid = adf.artefactdatafieldid
                WHERE ad.artefactid = ?
                ORDER BY ad.artefactdataid
            )";
            auto stmt = db.prepareStatement(query);
            stmt->setInt(1, pth.current_id());
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            file << "fieldtitle,value" << std::endl;

            int count = 0;
            while (res->next()) {
                file << escapeCSV(res->getString("fieldtitle")) << ","
                     << escapeCSV(res->getString("value")) << std::endl;
                count++;
            }
            std::cout << "Exported " << count << " artefact data items" << std::endl;

        } else if (pth.current_type() == Path::ARTEFACT_DATA_ITEM){
            throw NavigationError("Cannot export from an individual data item. Navigate to the Data folder.");

        } else if (pth.current_type() == Path::ASSOCIATIONS_FOLDER){
            // Export activity associations for the current artefact
            std::string query = R"(
                SELECT a.activityname, aal.ratio
                FROM ArtefactToActivityLink aal
                JOIN Activities a ON aal.activityid = a.activityid
                WHERE aal.artefactid = ?
                ORDER BY aal.artefactlinkid
            )";
            auto stmt = db.prepareStatement(query);
            stmt->setInt(1, pth.current_id());
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            file << "activityname,ratio" << std::endl;

            int count = 0;
            while (res->next()) {
                file << escapeCSV(res->getString("activityname")) << ","
                     << res->getDouble("ratio") << std::endl;
                count++;
            }
            std::cout << "Exported " << count << " activity associations" << std::endl;

        } else if (pth.current_type() == Path::ARTEFACT_TO_ACTIVITY_LINK){
            // Export milestone links for the current artefact-to-activity link
            std::string query = R"(
                SELECT ms.milestonestepname, mal.completedbyid, mal.completiondate
                FROM MilestoneToArtefactLink mal
                JOIN MilestoneSteps ms ON mal.milestonestepid = ms.milestonestepid
                WHERE mal.artefactlinkid = ?
                ORDER BY mal.milestonelinkid
            )";
            auto stmt = db.prepareStatement(query);
            stmt->setInt(1, pth.current_id());
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            file << "milestonestepname,completedbyid,completiondate" << std::endl;

            int count = 0;
            while (res->next()) {
                file << escapeCSV(res->getString("milestonestepname")) << ",";
                if (!res->isNull("completedbyid")) {
                    file << res->getInt("completedbyid");
                }
                file << ",";
                if (!res->isNull("completiondate")) {
                    file << res->getString("completiondate");
                }
                file << std::endl;
                count++;
            }
            std::cout << "Exported " << count << " milestone links" << std::endl;

        } else if (pth.current_type() == Path::MILESTONE_TO_ARTEFACT_LINK){
            throw NavigationError("Cannot export from an individual milestone link. Navigate to the Associations folder.");

        } else if (pth.current_type() == Path::MILESTONES_FOLDER){
            // Export milestones for the current project
            std::string query = "SELECT milestonename FROM Milestones WHERE projectid = ? ORDER BY milestoneid";
            auto stmt = db.prepareStatement(query);
            stmt->setInt(1, pth.current_id());
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            // Write header
            file << "milestonename" << std::endl;

            // Write data
            int count = 0;
            while (res->next()) {
                file << escapeCSV(res->getString("milestonename")) << std::endl;
                count++;
            }
            std::cout << "Exported " << count << " milestones" << std::endl;

        } else if (pth.current_type() == Path::MILESTONE){
            // Export milestone steps for the current milestone
            std::string query = R"(
                SELECT milestonestepname, progressratio
                FROM MilestoneSteps
                WHERE milestoneid = ?
                ORDER BY progressratio, milestonestepid
            )";
            auto stmt = db.prepareStatement(query);
            stmt->setInt(1, pth.current_id());
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            // Write header
            file << "milestonestepname,progressratio" << std::endl;

            // Write data
            int count = 0;
            while (res->next()) {
                file << escapeCSV(res->getString("milestonestepname")) << ","
                     << res->getDouble("progressratio") << std::endl;
                count++;
            }
            std::cout << "Exported " << count << " milestone steps" << std::endl;

        } else if (pth.current_type() == Path::ASSIGNMENTS_FOLDER){
            // Export assignments for the current task
            std::string query = "SELECT userid, assigneddate, closedate FROM ActivityTaskAssignments WHERE taskid = ? ORDER BY assignmentid";
            auto stmt = db.prepareStatement(query);
            stmt->setInt(1, pth.current_id());
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            // Write header
            file << "userid,assigneddate,closedate" << std::endl;

            // Write data
            int count = 0;
            while (res->next()) {
                file << res->getInt("userid") << ",";
                if (!res->isNull("assigneddate")) {
                    file << res->getString("assigneddate");
                }
                file << ",";
                if (!res->isNull("closedate")) {
                    file << res->getString("closedate");
                }
                file << std::endl;
                count++;
            }
            std::cout << "Exported " << count << " assignments" << std::endl;

        } else if (pth.current_type() == Path::ASSIGNMENT){
            // Export hours booked for the current assignment
            std::string query = "SELECT bookeddate, hours FROM Hours WHERE assignmentid = ? ORDER BY bookingid";
            auto stmt = db.prepareStatement(query);
            stmt->setInt(1, pth.current_id());
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            // Write header
            file << "bookeddate,hours" << std::endl;

            // Write data
            int count = 0;
            while (res->next()) {
                file << res->getString("bookeddate") << ","
                     << res->getDouble("hours") << std::endl;
                count++;
            }
            std::cout << "Exported " << count << " hours bookings" << std::endl;

        } else if (pth.current_type() == Path::HOURS){
            throw NavigationError("Cannot export from an individual hours booking. Navigate to the Assignments folder.");
        } else if (pth.current_type() == Path::EMPLOYEE){
            throw NavigationError("Cannot export from an individual employee. Navigate to the Department.");
        } else if (pth.current_type() == Path::ACTIVITY_NOTE){
            throw NavigationError("Cannot export from an individual note. Navigate to the Notes folder.");
        } else if (pth.current_type() == Path::ARTEFACT){
            throw NavigationError("Cannot export here. Navigate to the Data or Associations folder.");
        } else if (pth.current_type() == Path::ARTEFACT_FIELD){
            throw NavigationError("Cannot export from an individual field. Navigate to the Fields folder.");
        } else if (pth.current_type() == Path::MILESTONE_STEP){
            throw NavigationError("Cannot export from an individual milestone step. Navigate to the Milestone.");
        } else {
            throw NavigationError("Cannot export from this location.");
        }

    } catch (sql::SQLException& e) {
        file.close();
        throw DatabaseError("export", e.what());
    }

    file.close();
    std::cout << "Export completed successfully." << std::endl;
}

} //namespace commands
