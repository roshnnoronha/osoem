#include "set.h"
#include "../exceptions/exceptions.h"
#include "../utils/auth.h"
#include "../utils/csv_parser.h"
#include <iostream>

namespace commands {

static int resolveItemId(Database& db, Path& pth, const std::string& param) {
    try {
        switch (pth.current_type()) {
            case Path::DEPARTMENTS_FOLDER:
                return utils::resolveIdOrName(db, param, utils::lookupDepartment);
            case Path::DEPARTMENT:
                return utils::resolveIdOrName(db, param, utils::lookupEmployee);
            case Path::PROJECTS_FOLDER:
                return utils::resolveIdOrName(db, param, utils::lookupProject);
            case Path::ACTIVITIES_FOLDER:
                return utils::resolveIdOrNameScoped(db, param, pth.current_id(), utils::lookupCategory);
            case Path::CATEGORY:
                return utils::resolveIdOrNameScoped(db, param, pth.current_id(), utils::lookupSubcategory);
            case Path::SUBCATEGORY:
                return utils::resolveIdOrNameScoped(db, param, pth.current_id(), utils::lookupActivityBySubcategory);
            case Path::TASKS_FOLDER:
                if (pth.getIdByType(Path::TASK) > 0)
                    return utils::resolveIdOrNameScoped(db, param, pth.current_id(), utils::lookupTaskByParent);
                else
                    return utils::resolveIdOrNameScoped(db, param, pth.current_id(), utils::lookupTaskByActivity);
            case Path::NOTES_FOLDER:
                return utils::resolveIdOrNameScoped(db, param, pth.current_id(), utils::lookupNote);
            case Path::ARTEFACT_TYPES_FOLDER:
                return utils::resolveIdOrNameScoped(db, param, pth.current_id(), utils::lookupArtefactType);
            case Path::ARTEFACTS_FOLDER:
                return utils::resolveIdOrNameScoped(db, param, pth.current_id(), utils::lookupArtefact);
            case Path::FIELDS_FOLDER:
                return utils::resolveIdOrNameScoped(db, param, pth.current_id(), utils::lookupArtefactDataField);
            case Path::MILESTONES_FOLDER:
                return utils::resolveIdOrNameScoped(db, param, pth.current_id(), utils::lookupMilestone);
            case Path::MILESTONE:
                return utils::resolveIdOrNameScoped(db, param, pth.current_id(), utils::lookupMilestoneStep);
            case Path::TEAM_FOLDER:
                return utils::resolveIdOrNameScoped(db, param, pth.getIdByType(Path::PROJECT), utils::lookupTeamMember);
            case Path::ASSIGNMENTS_FOLDER:
                return utils::resolveIdOrNameScoped(db, param, pth.current_id(), utils::lookupAssignment);
            case Path::DATA_FOLDER:
                return utils::resolveIdOrNameScoped(db, param, pth.getIdByType(Path::ARTEFACT), utils::lookupArtefactDataItem);
            case Path::ASSOCIATIONS_FOLDER:
                return utils::resolveIdOrNameScoped(db, param, pth.getIdByType(Path::ARTEFACT), utils::lookupArtefactActivityLink);
            case Path::ARTEFACT_TO_ACTIVITY_LINK:
                return utils::resolveIdOrNameScoped(db, param, pth.current_id(), utils::lookupMilestoneToArtefactLink);
            case Path::ASSIGNMENT:
                return utils::resolveIdOrNameScoped(db, param, pth.current_id(), utils::lookupHours);
            default:
                throw ValidationError("Invalid ID: '" + param + "'. Must be an integer.");
        }
    } catch (sql::SQLException& e) {
        throw DatabaseError("resolving item ID", e.what());
    }
}

void set (Database& db, Path& pth, std::string id, std::string csvValues){
    switch(pth.current_type()) {
        case Path::ROOT:
        case Path::ORGANIZATION_FOLDER:
        case Path::PROJECT:
        case Path::ACTIVITY:
        case Path::TASK:
        case Path::ARTEFACT_TYPE:
            throw NavigationError("Cannot edit an item here.");
    } 

    // Parse ID (accepts integer or name)
    int itemId = resolveItemId(db, pth, id);

    // Parse CSV string
    std::vector<std::map<std::string,std::string>> CSVData;
    try {
        CSVData = utils::parseCSVString(csvValues);
    } catch (const std::invalid_argument& e) {
        throw ValidationError(std::string("Invalid format: ") + e.what() + ". Use: st <id> \"fieldname:value, ...\"");
    }

    if (CSVData.empty()) {
        throw ValidationError("No values provided.");
    }

    auto& row = CSVData[0];

    try {
        switch (pth.current_type()) {
            case Path::DEPARTMENTS_FOLDER: {
                DepartmentData dept(itemId, &db);
                for (auto& [key, val] : row) dept.setValue(key, val);
                setDepartmentData(db, dept, itemId);
                break;
            }
            case Path::DEPARTMENT: {
                EmployeeData emp(itemId, &db);
                for (auto& [key, val] : row) emp.setValue(key, val);
                setEmployeeData(db, emp, itemId);
                break;
            }
            case Path::PROJECTS_FOLDER: {
                ProjectData proj(itemId, &db);
                for (auto& [key, val] : row) proj.setValue(key, val);
                setProjectData(db, proj, itemId);
                break;
            }
            case Path::ACTIVITIES_FOLDER: {
                CategoryData cat(itemId, &db);
                for (auto& [key, val] : row) cat.setValue(key, val);
                setCategoryData(db, cat, itemId);
                break;
            }
            case Path::CATEGORY: {
                SubCategoryData subcat(itemId, &db);
                for (auto& [key, val] : row) subcat.setValue(key, val);
                setSubcategoryData(db, subcat, itemId);
                break;
            }
            case Path::SUBCATEGORY: {
                ActivityData act(itemId, &db);
                for (auto& [key, val] : row) act.setValue(key, val);
                setActivityData(db, act, itemId);
                break;
            }
            case Path::TASKS_FOLDER: {
                TaskData task( itemId, &db);
                for (auto& [key, val] : row) task.setValue(key, val);
                setTaskData(db, task, itemId);
                break;
            }
            case Path::NOTES_FOLDER: {
                NoteData note(itemId, &db);
                for (auto& [key, val] : row) note.setValue(key, val);
                setNoteData(db, note, itemId);
                break;
            }
            case Path::ARTEFACT_TYPES_FOLDER: {
                ArtefactTypeData artType(itemId, &db);
                for (auto& [key, val] : row) artType.setValue(key, val);
                setArtefactTypeData(db, artType, itemId);
                break;
            }
            case Path::ARTEFACTS_FOLDER: {
                ArtefactData art(itemId, &db);
                for (auto& [key, val] : row) art.setValue(key, val);
                setArtefactData(db, art, itemId);
                break;
            }
            case Path::FIELDS_FOLDER: {
                FieldData field(itemId, &db);
                for (auto& [key, val] : row) field.setValue(key, val);
                setFieldData(db, field, itemId);
                break;
            }
            case Path::MILESTONES_FOLDER: {
                MilestoneData milestone(itemId, &db);
                for (auto& [key, val] : row) milestone.setValue(key, val);
                setMilestoneData(db, milestone, itemId);
                break;
            }
            case Path::MILESTONE: {
                MilestoneStepData step(itemId, &db);
                for (auto& [key, val] : row) step.setValue(key, val);
                setMilestoneStepData(db, step, itemId);
                break;
            }
            case Path::TEAM_FOLDER: {
                ProjectTeamMemberData member(itemId, &db);
                for (auto& [key, val] : row) member.setValue(key, val);
                setTeamMemberData(db, member, itemId);
                break;
            }
            case Path::ASSIGNMENTS_FOLDER: {
                AssignmentData assignment(itemId, &db);
                for (auto& [key, val] : row) assignment.setValue(key, val);
                setAssignmentData(db, assignment, itemId);
                break;
            }
            case Path::ASSIGNMENT: {
                HoursData hours(itemId, &db);
                for (auto& [key, val] : row) hours.setValue(key, val);
                setHoursData(db, hours, itemId);
                break;
            }
            case Path::DATA_FOLDER: {
                ArtefactDataItemData data(itemId, &db);
                for (auto& [key, val] : row) data.setValue(key, val);
                setArtefactDataItemData(db, data, itemId);
                break;
            }
            case Path::ASSOCIATIONS_FOLDER: {
                ArtefactToActivityLinkData link(itemId, &db);
                for (auto& [key, val] : row) link.setValue(key, val);
                setArtefactToActivityLinkData(db, link, itemId);
                break;
            }
            case Path::ARTEFACT_TO_ACTIVITY_LINK: {
                MilestoneToArtefactLinkData link(itemId, &db);
                for (auto& [key, val] : row) link.setValue(key, val);
                setMilestoneToArtefactLinkData(db, link, itemId);
            }
        }
    } catch (const DataError& e) {
        throw;
    }
}

void setEmployeeData (Database& db, EmployeeData& emp, int employeeId){
    try {
        // Verify employee exists and get current password salt
        auto checkStmt = db.prepareStatement("SELECT employeeid, password, passwordsalt FROM Employees WHERE employeeid = ?");
        checkStmt->setInt(1, employeeId);
        auto checkRes = checkStmt->executeQuery();

        if (!checkRes->next()) {
            throw EntityNotFoundError("Employee", employeeId);
        }

        std::string currentStoredPassword = checkRes->getString("password");
        std::string currentStoredSalt = checkRes->getString("passwordsalt");

        // Check if password is being changed
        std::string newPassword = emp.getPassword();
        std::string passwordToStore = currentStoredPassword;
        std::string saltToStore = currentStoredSalt;

        // If password field has changed (not the same as what's stored), rehash it
        if (newPassword != currentStoredPassword) {
            // Generate new salt and hash the password
            saltToStore = auth::generateSalt();
            passwordToStore = auth::hashPassword(newPassword, saltToStore);
        }

        // Build and execute UPDATE query
        std::string query = "UPDATE Employees SET firstname = ?, lastname = ?, email = ?, password = ?, passwordsalt = ?, admin = ? WHERE employeeid = ?";
        auto updateStmt = db.prepareStatement(query);
        updateStmt->setString(1, emp.getFirstName());
        updateStmt->setString(2, emp.getLastName());
        updateStmt->setString(3, emp.getEmail());
        updateStmt->setString(4, passwordToStore);
        updateStmt->setString(5, saltToStore);
        updateStmt->setBoolean(6, emp.isAdmin());
        updateStmt->setInt(7, employeeId);

        int rowsAffected = updateStmt->executeUpdate();

        if (rowsAffected > 0) {
            std::cout << "Successfully updated employee details." << std::endl;
        } 
    } catch (sql::SQLException& e) {
        throw DatabaseError("updating employee data", e.what());
    }
}

void setProjectData(Database& db, ProjectData& proj, int projectId) {
    try {
        // Verify project exists
        auto checkStmt = db.prepareStatement("SELECT projectid FROM Projects WHERE projectid = ?");
        checkStmt->setInt(1, projectId);
        auto checkRes = checkStmt->executeQuery();

        if (!checkRes->next()) {
            throw EntityNotFoundError("Project", projectId);
        }

        // Build and execute UPDATE query
        std::string query = "UPDATE Projects SET projectname = ?, projectno = ? WHERE projectid = ?";
        auto updateStmt = db.prepareStatement(query);
        updateStmt->setString(1, proj.getName());
        updateStmt->setString(2, proj.getNumber());
        updateStmt->setInt(3, projectId);

        int rowsAffected = updateStmt->executeUpdate();

        if (rowsAffected > 0) {
            std::cout << "Successfully updated project details." << std::endl;
        } 
    } catch (sql::SQLException& e) {
        throw DatabaseError("updating project data", e.what());
    }
}

void setActivityData(Database& db, ActivityData& act, int activityId) {
    try {
        // Verify activity exists
        auto checkStmt = db.prepareStatement("SELECT activityid FROM Activities WHERE activityid = ?");
        checkStmt->setInt(1, activityId);
        auto checkRes = checkStmt->executeQuery();

        if (!checkRes->next()) {
            throw EntityNotFoundError("Activity", activityId);
        }

        // Build and execute UPDATE query
        std::string query = "UPDATE Activities SET activityname = ?, activitydescription = ?, activitymanagerid = ?, subcategoryid = ?, plannedstart = ?, plannedfinish = ?, forecaststart = ?, forecastfinish = ?, actualfinish = ?, plannedhours = ?, forecasthours = ? WHERE activityid = ?";
        auto updateStmt = db.prepareStatement(query);
        updateStmt->setString(1, act.getName());
        updateStmt->setString(2, act.getDescription());
        updateStmt->setInt(3, act.getManagerId());
        updateStmt->setInt(4, act.getSubCategoryId());
        if (act.getPlannedStart().empty())
            updateStmt->setNull(5, sql::DataType::DATE);
        else
            updateStmt->setString(5, act.getPlannedStart());
        if (act.getPlannedEnd().empty())
            updateStmt->setNull(6, sql::DataType::DATE);
        else
            updateStmt->setString(6, act.getPlannedEnd());
        if (act.getForecastStart().empty())
            updateStmt->setNull(7, sql::DataType::DATE);
        else
            updateStmt->setString(7, act.getForecastStart());
        if (act.getForecastEnd().empty())
            updateStmt->setNull(8, sql::DataType::DATE);
        else
            updateStmt->setString(8, act.getForecastEnd());
        if (act.getActualEnd().empty())
            updateStmt->setNull(9, sql::DataType::DATE);
        else
            updateStmt->setString(9, act.getActualEnd());
        updateStmt->setDouble(10, act.getPlannedHours());
        updateStmt->setDouble(11, act.getForecastHours());
        updateStmt->setInt(12, activityId);

        int rowsAffected = updateStmt->executeUpdate();

        if (rowsAffected > 0) {
            std::cout << "Successfully updated activity details." << std::endl;
        } 
    } catch (sql::SQLException& e) {
        throw DatabaseError("updating activity data", e.what());
    }
}

void setCategoryData(Database& db, CategoryData& cat, int categoryId) {
    try {
        // Verify category exists
        auto checkStmt = db.prepareStatement("SELECT categoryid FROM ActivityCategories WHERE categoryid = ?");
        checkStmt->setInt(1, categoryId);
        auto checkRes = checkStmt->executeQuery();

        if (!checkRes->next()) {
            throw EntityNotFoundError("Category", categoryId);
        }

        // Build and execute UPDATE query
        std::string query = "UPDATE ActivityCategories SET categoryname = ?, projectid = ? WHERE categoryid = ?";
        auto updateStmt = db.prepareStatement(query);
        updateStmt->setString(1, cat.getName());
        updateStmt->setInt(2, cat.getProjectId());
        updateStmt->setInt(3, categoryId);

        int rowsAffected = updateStmt->executeUpdate();

        if (rowsAffected > 0) {
            std::cout << "Successfully updated category details." << std::endl;
        } 
    } catch (sql::SQLException& e) {
        throw DatabaseError("updating category data", e.what());
    }
}

void setSubcategoryData(Database& db, SubCategoryData& subcat, int subcategoryId) {
    try {
        // Verify subcategory exists
        auto checkStmt = db.prepareStatement("SELECT subcategoryid FROM ActivitySubcategories WHERE subcategoryid = ?");
        checkStmt->setInt(1, subcategoryId);
        auto checkRes = checkStmt->executeQuery();

        if (!checkRes->next()) {
            throw EntityNotFoundError("Subcategory", subcategoryId);
        }

        // Build and execute UPDATE query
        std::string query = "UPDATE ActivitySubcategories SET subcategoryname = ?, categoryid = ? WHERE subcategoryid = ?";
        auto updateStmt = db.prepareStatement(query);
        updateStmt->setString(1, subcat.getName());
        updateStmt->setInt(2, subcat.getCategoryId());
        updateStmt->setInt(3, subcategoryId);

        int rowsAffected = updateStmt->executeUpdate();

        if (rowsAffected > 0) {
            std::cout << "Successfully updated subcategory details." << std::endl;
        } 
    } catch (sql::SQLException& e) {
        throw DatabaseError("updating subcategory data", e.what());
    }
}

void setTaskData(Database& db, TaskData& task, int taskId) {
    try {
        // Verify task exists
        auto checkStmt = db.prepareStatement("SELECT taskid FROM ActivityTasks WHERE taskid = ?");
        checkStmt->setInt(1, taskId);
        auto checkRes = checkStmt->executeQuery();

        if (!checkRes->next()) {
            throw EntityNotFoundError("Task", taskId);
        }

        // Build and execute UPDATE query
        std::string query = "UPDATE ActivityTasks SET taskname = ?, taskdescription = ?, activityid = ?, parenttaskid = ?, departmentid = ? WHERE taskid = ?";
        auto updateStmt = db.prepareStatement(query);
        updateStmt->setString(1, task.getName());
        updateStmt->setString(2,task.getDescription());
        updateStmt->setInt(3,task.getActivityId());
        if (task.getParentTaskId() == 0)
            updateStmt->setNull(4, sql::DataType::INTEGER);
        else
            updateStmt->setInt(4,task.getParentTaskId());
        if (task.getDepartmentId() == 0)
            updateStmt->setNull(5, sql::DataType::INTEGER);
        else
            updateStmt->setInt(5, task.getDepartmentId());
        updateStmt->setInt(6, taskId);

        int rowsAffected = updateStmt->executeUpdate();

        if (rowsAffected > 0) {
            std::cout << "Successfully updated task details." << std::endl;
        } 
    } catch (sql::SQLException& e) {
        throw DatabaseError("updating task data", e.what());
    }
}

void setArtefactTypeData(Database& db, ArtefactTypeData& artType, int artefactTypeId) {
    try {
        // Verify artefact type exists
        auto checkStmt = db.prepareStatement("SELECT artefacttypeid FROM ArtefactTypes WHERE artefacttypeid = ?");
        checkStmt->setInt(1, artefactTypeId);
        auto checkRes = checkStmt->executeQuery();

        if (!checkRes->next()) {
            throw EntityNotFoundError("Artefact type", artefactTypeId);
        }

        // Build and execute UPDATE query
        std::string query = "UPDATE ArtefactTypes SET artefactname = ?, artefactdescription = ?, projectid = ? WHERE artefacttypeid = ?";
        auto updateStmt = db.prepareStatement(query);
        updateStmt->setString(1, artType.getName());
        updateStmt->setString(2, artType.getDescription());
        updateStmt->setInt(3, artType.getProjectId());
        updateStmt->setInt(4, artefactTypeId);

        int rowsAffected = updateStmt->executeUpdate();

        if (rowsAffected > 0) {
            std::cout << "Successfully updated artefact type details." << std::endl;
        } 
    } catch (sql::SQLException& e) {
        throw DatabaseError("updating artefact type data", e.what());
    }
}

void setArtefactData(Database& db, ArtefactData& art, int artefactId) {
    try {
        // Verify artefact exists
        auto checkStmt = db.prepareStatement("SELECT artefactid FROM Artefacts WHERE artefactid = ?");
        checkStmt->setInt(1, artefactId);
        auto checkRes = checkStmt->executeQuery();

        if (!checkRes->next()) {
            throw EntityNotFoundError("Artefact", artefactId);
        }

        // Build and execute UPDATE query
        std::string query = "UPDATE Artefacts SET artefacttitle = ?, artefactownerid = ? WHERE artefactid = ?";
        auto updateStmt = db.prepareStatement(query);
        updateStmt->setString(1, art.getName());
        updateStmt->setInt(2, art.getArtefactOwnerId());
        updateStmt->setInt(3, artefactId);

        int rowsAffected = updateStmt->executeUpdate();

        if (rowsAffected > 0) {
            std::cout << "Successfully updated artefact details." << std::endl;
        } 
    } catch (sql::SQLException& e) {
        throw DatabaseError("updating artefact data", e.what());
    }
}

void setNoteData(Database& db, NoteData& note, int noteId) {
    try {
        // Verify note exists
        auto checkStmt = db.prepareStatement("SELECT noteid FROM ActivityNotes WHERE noteid = ?");
        checkStmt->setInt(1, noteId);
        auto checkRes = checkStmt->executeQuery();

        if (!checkRes->next()) {
            throw EntityNotFoundError("Note", noteId);
        }

        // Build and execute UPDATE query
        std::string query = "UPDATE ActivityNotes SET note = ?, notedate = ?, userid = ? WHERE noteid = ?";
        auto updateStmt = db.prepareStatement(query);
        updateStmt->setString(1, note.getNote());
        updateStmt->setString(2, note.getNoteDate());
        updateStmt->setInt(3, note.getUserId());
        updateStmt->setInt(4, noteId);

        int rowsAffected = updateStmt->executeUpdate();

        if (rowsAffected > 0) {
            std::cout << "Successfully updated note details." << std::endl;
        } 
    } catch (sql::SQLException& e) {
        throw DatabaseError("updating note data", e.what());
    }
}

void setFieldData(Database& db, FieldData& field, int fieldId) {
    try {
        // Verify field exists
        auto checkStmt = db.prepareStatement("SELECT artefactdatafieldid FROM ArtefactDataFields WHERE artefactdatafieldid = ?");
        checkStmt->setInt(1, fieldId);
        auto checkRes = checkStmt->executeQuery();

        if (!checkRes->next()) {
            throw EntityNotFoundError("Field", fieldId);
        }

        // Build and execute UPDATE query
        std::string query = "UPDATE ArtefactDataFields SET fieldtitle = ?, valuetype = ?, maximumlength = ?, maximumvalue = ?, minimumvalue = ? WHERE artefactdatafieldid = ?";
        auto updateStmt = db.prepareStatement(query);
        updateStmt->setString(1, field.getTitle());
        updateStmt->setInt(2, field.getValueType());
        updateStmt->setInt(3, field.getMaimumLength());
        updateStmt->setInt(4, field.getMaximumValue());
        updateStmt->setInt(5, field.getMinimumValue());
        updateStmt->setInt(6, fieldId);

        int rowsAffected = updateStmt->executeUpdate();

        if (rowsAffected > 0) {
            std::cout << "Successfully updated field details." << std::endl;
        } 
    } catch (sql::SQLException& e) {
        throw DatabaseError("updating field data", e.what());
    }
}

void setMilestoneData(Database& db, MilestoneData& milestone, int milestoneId) {
    try {
        // Verify milestone exists
        auto checkStmt = db.prepareStatement("SELECT milestoneid FROM Milestones WHERE milestoneid = ?");
        checkStmt->setInt(1, milestoneId);
        auto checkRes = checkStmt->executeQuery();

        if (!checkRes->next()) {
            throw EntityNotFoundError("Milestone", milestoneId);
        }

        // Build and execute UPDATE query
        std::string query = "UPDATE Milestones SET milestonename = ?, projectid = ? WHERE milestoneid = ?";
        auto updateStmt = db.prepareStatement(query);
        updateStmt->setString(1, milestone.getName());
        updateStmt->setInt(2, milestone.getProjectId());
        updateStmt->setInt(3, milestoneId);

        int rowsAffected = updateStmt->executeUpdate();

        if (rowsAffected > 0) {
            std::cout << "Successfully updated milestone details." << std::endl;
        } 
    } catch (sql::SQLException& e) {
        throw DatabaseError("updating milestone data", e.what());
    }
}

void setMilestoneStepData(Database& db, MilestoneStepData& step, int milestoneStepId) {
    try {
        // Verify milestone step exists
        auto checkStmt = db.prepareStatement("SELECT milestonestepid FROM MilestoneSteps WHERE milestonestepid = ?");
        checkStmt->setInt(1, milestoneStepId);
        auto checkRes = checkStmt->executeQuery();

        if (!checkRes->next()) {
            throw EntityNotFoundError("Milestone step", milestoneStepId);
        }

        // Build and execute UPDATE query
        std::string query = "UPDATE MilestoneSteps SET milestonestepname = ?, progressratio = ?, milestoneid = ? WHERE milestonestepid = ?";
        auto updateStmt = db.prepareStatement(query);
        updateStmt->setString(1, step.getName());
        updateStmt->setDouble(2, step.getProgressRatio());
        updateStmt->setInt(3, step.getMilestoneId());
        updateStmt->setInt(4, milestoneStepId);

        int rowsAffected = updateStmt->executeUpdate();

        if (rowsAffected > 0) {
            std::cout << "Successfully updated milestone step details." << std::endl;
        } 
    } catch (sql::SQLException& e) {
        throw DatabaseError("updating milestone step data", e.what());
    }
}

void setDepartmentData(Database& db, DepartmentData& dept, int departmentId) {
    try {
        // Verify department exists
        auto checkStmt = db.prepareStatement("SELECT departmentid FROM Departments WHERE departmentid = ?");
        checkStmt->setInt(1, departmentId);
        auto checkRes = checkStmt->executeQuery();

        if (!checkRes->next()) {
            throw EntityNotFoundError("Department", departmentId);
        }

        // Build and execute UPDATE query
        std::string query = "UPDATE Departments SET departmentname = ? WHERE departmentid = ?";
        auto updateStmt = db.prepareStatement(query);
        updateStmt->setString(1, dept.getName());
        updateStmt->setInt(2, departmentId);

        int rowsAffected = updateStmt->executeUpdate();

        if (rowsAffected > 0) {
            std::cout << "Successfully updated department details." << std::endl;
        } 
    } catch (sql::SQLException& e) {
        throw DatabaseError("updating department data", e.what());
    }
}

void setTeamMemberData(Database& db, ProjectTeamMemberData& member, int teamMemberId) {
    try {
        // Verify team member exists
        auto checkStmt = db.prepareStatement("SELECT teammemberid FROM ProjectTeamMembers WHERE teammemberid = ?");
        checkStmt->setInt(1, teamMemberId);
        auto checkRes = checkStmt->executeQuery();

        if (!checkRes->next()) {
            throw EntityNotFoundError("Team member", teamMemberId);
        }

        // Build and execute UPDATE query
        std::string query = "UPDATE ProjectTeamMembers SET employeeid = ?, role = ? WHERE teammemberid = ?";
        auto updateStmt = db.prepareStatement(query);
        updateStmt->setInt(1, member.getEmployeeId());
        updateStmt->setInt(2, member.getRole());
        updateStmt->setInt(3, teamMemberId);

        int rowsAffected = updateStmt->executeUpdate();

        if (rowsAffected > 0) {
            std::cout << "Successfully updated team member details." << std::endl;
        } 
    } catch (sql::SQLException& e) {
        throw DatabaseError("updating team member data", e.what());
    }
}

void setAssignmentData(Database& db, AssignmentData& assignment, int assignmentId) {
    try {
        // Verify assignment exists
        auto checkStmt = db.prepareStatement("SELECT assignmentid FROM ActivityTaskAssignments WHERE assignmentid = ?");
        checkStmt->setInt(1, assignmentId);
        auto checkRes = checkStmt->executeQuery();

        if (!checkRes->next()) {
            throw EntityNotFoundError("Assignment", assignmentId);
        }

        // Build and execute UPDATE query
        std::string query = "UPDATE ActivityTaskAssignments SET userid = ?, assigneddate = ?, closedate = ? WHERE assignmentid = ?";
        auto updateStmt = db.prepareStatement(query);
        updateStmt->setInt(1, assignment.getUserId());
        updateStmt->setString(2, assignment.getAssignedDate());
        if (assignment.getCloseDate().empty()) {
            updateStmt->setNull(3, sql::DataType::DATE);
        } else {
            updateStmt->setString(3, assignment.getCloseDate());
        }
        updateStmt->setInt(4, assignmentId);

        int rowsAffected = updateStmt->executeUpdate();

        if (rowsAffected > 0) {
            std::cout << "Successfully updated assignment details." << std::endl;
        } 
    } catch (sql::SQLException& e) {
        throw DatabaseError("updating assignment data", e.what());
    }
}

void setHoursData(Database& db, HoursData& hours, int bookingId) {
    try {
        // Verify hours entry exists
        auto checkStmt = db.prepareStatement("SELECT bookingid FROM Hours WHERE bookingid = ?");
        checkStmt->setInt(1, bookingId);
        auto checkRes = checkStmt->executeQuery();

        if (!checkRes->next()) {
            throw EntityNotFoundError("Hours entry", bookingId);
        }

        // Build and execute UPDATE query
        std::string query = "UPDATE Hours SET bookeddate = ?, hours = ? WHERE bookingid = ?";
        auto updateStmt = db.prepareStatement(query);
        updateStmt->setString(1, hours.getBookedDate());
        updateStmt->setInt(2, hours.getHours());
        updateStmt->setInt(3, bookingId);

        int rowsAffected = updateStmt->executeUpdate();

        if (rowsAffected > 0) {
            std::cout << "Successfully updated hours details." << std::endl;
        } 
    } catch (sql::SQLException& e) {
        throw DatabaseError("updating hours data", e.what());
    }
}

void setArtefactDataItemData(Database& db, ArtefactDataItemData& data, int artefactDataId) {
    try {
        // Verify artefact data item exists
        auto checkStmt = db.prepareStatement("SELECT artefactdataid FROM ArtefactData WHERE artefactdataid = ?");
        checkStmt->setInt(1, artefactDataId);
        auto checkRes = checkStmt->executeQuery();

        if (!checkRes->next()) {
            throw EntityNotFoundError("Artefact data item", artefactDataId);
        }

        // Build and execute UPDATE query
        std::string query = "UPDATE ArtefactData SET artefactdatafieldid = ?, value = ? WHERE artefactdataid = ?";
        auto updateStmt = db.prepareStatement(query);
        updateStmt->setInt(1, data.getArtefactDataFieldId());
        updateStmt->setString(2, data.getValue());
        updateStmt->setInt(3, artefactDataId);

        int rowsAffected = updateStmt->executeUpdate();

        if (rowsAffected > 0) {
            std::cout << "Successfully updated artefact data item details." << std::endl;
        } 
    } catch (sql::SQLException& e) {
        throw DatabaseError("updating artefact data item", e.what());
    }
}

void setArtefactToActivityLinkData(Database& db, ArtefactToActivityLinkData& link, int artefactLinkId) {
    try {
        // Verify artefact to activity link exists
        auto checkStmt = db.prepareStatement("SELECT artefactlinkid FROM ArtefactToActivityLink WHERE artefactlinkid = ?");
        checkStmt->setInt(1, artefactLinkId);
        auto checkRes = checkStmt->executeQuery();

        if (!checkRes->next()) {
            throw EntityNotFoundError("Artefact to activity link", artefactLinkId);
        }

        // Build and execute UPDATE query
        std::string query = "UPDATE ArtefactToActivityLink SET activityid = ?, ratio = ? WHERE artefactlinkid = ?";
        auto updateStmt = db.prepareStatement(query);
        updateStmt->setInt(1, link.getActivityId());
        updateStmt->setDouble(2, link.getRatio());
        updateStmt->setInt(3, artefactLinkId);

        int rowsAffected = updateStmt->executeUpdate();

        if (rowsAffected > 0) {
            std::cout << "Successfully updated artefact to activity link details." << std::endl;
        } 
    } catch (sql::SQLException& e) {
        throw DatabaseError("updating artefact to activity link data", e.what());
    }
}

void setMilestoneToArtefactLinkData(Database& db, MilestoneToArtefactLinkData& link, int milestoneLinkId) {
    try {
        // Verify milestone to artefact link exists
        auto checkStmt = db.prepareStatement("SELECT milestonelinkid FROM MilestoneToArtefactLink WHERE milestonelinkid = ?");
        checkStmt->setInt(1, milestoneLinkId);
        auto checkRes = checkStmt->executeQuery();

        if (!checkRes->next()) {
            throw EntityNotFoundError("Milestone to artefact link", milestoneLinkId);
        }

        // Build and execute UPDATE query
        std::string query = "UPDATE MilestoneToArtefactLink SET milestonestepid = ?, completedby = ?, completiondate = ? WHERE milestonelinkid = ?";
        auto updateStmt = db.prepareStatement(query);
        updateStmt->setInt(1, link.getMilestoneStepId());
        if (link.getCompletedById() == 0) {
            updateStmt->setNull(2, sql::DataType::INTEGER);
        } else {
            updateStmt->setInt(2, link.getCompletedById());
        }
        if (link.getCompletionDate().empty()) {
            updateStmt->setNull(3, sql::DataType::DATE);
        } else {
            updateStmt->setString(3, link.getCompletionDate());
        }
        updateStmt->setInt(4, milestoneLinkId);

        int rowsAffected = updateStmt->executeUpdate();

        if (rowsAffected > 0) {
            std::cout << "Successfully updated milestone to artefact link details." << std::endl;
        } 
    } catch (sql::SQLException& e) {
        throw DatabaseError("updating milestone to artefact link data", e.what());
    }
}

} // namespace commands

