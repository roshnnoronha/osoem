#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "database.h"
#include "utils.h"
#include "path.h"
#include "commands/list.h"
#include "commands/select.h"
#include "commands/add.h"
#include "commands/remove.h"
#include "commands/employee.h"

int main(int argc, char* argv[]) {


    try {
        // Initialize database connection
        Database db;

        if (!db.isConnected()) {
            std::cerr << "Failed to connect to database." << std::endl;
            return 1;
        }

        // Check if the utility is opened in interactive mode
        if (argc < 2) {
            std::string inp;
            std::string tok;
            std::vector<std::string> tokens;
            std::string command;
            std::stringstream ssInp;
            std::string param;
            Path pth;
            while (inp != "exit"){
                std::cout << pth.to_string() << ">";
                std::getline(std::cin,inp);
                ssInp.clear();
                ssInp.str("");
                ssInp << inp;
                tokens.clear();
                int id = 0;
                while (std::getline(ssInp, tok, ' ')){
                    if (!tok.empty()) {
                        tokens.push_back(tok);
                    }
                }

                if (tokens.empty()) continue;

                command = tokens[0];
                param = (tokens.size() > 1) ? tokens[1] : "";

                // Handle special commands
                if (command == "exit") break;
                if (command == ".." || param == "..") {
                    pth.move_back();
                    continue;
                }

                // Parse ID from param if needed
                if (!param.empty() && utils::isNumber(param)) {
                    id = utils::toInt(param);
                }
                if ((command == "ls") || (command == "list")){
                    switch(pth.current_type()){
                        case Path::ROOT:
                            std::cout << "\nAvailable items:" << std::endl;
                            std::cout << "  - Employees (use 'sl employees')" << std::endl;
                            std::cout << "  - Projects (use 'ls projects')" << std::endl;
                            break;
                        case Path::EMPLOYEES:
                            commands::employee::listEmployees(db);
                            break;
                        case Path::PROJECT:
                            commands::list::listCategories(db,pth.current_id());
                            break;
                        case Path::CATEGORY:
                            commands::list::listSubcategories(db,pth.current_id());
                            break;
                        case Path::SUBCATEGORY:
                            commands::list::listActivities(db, pth.current_id());
                            break;
                        case Path::ACTIVITY:
                            commands::list::listTasks(db, pth.current_id());
                            break;
                        case Path::TASK:
                            commands::list::listSubTasks(db, pth.current_id());
                            break;
                    }
                }
                if (command == "ls" && param == "projects") {
                    commands::list::listProjects(db);
                }
                if ((command == "sl") || (command == "select")){
                    if (pth.current_type() == Path::ROOT && param == "employees") {
                        // Navigate to employees virtual folder
                        pth.move_forward("Employees", 0, Path::EMPLOYEES);
                    } else {
                        switch(pth.current_type()){
                            case Path::ROOT:
                                commands::select::selectProject(db, pth, id);
                                break;
                            case Path::EMPLOYEES:
                                commands::employee::selectEmployee(db, pth, id);
                                break;
                            case Path::PROJECT:
                                commands::select::selectCategory(db,pth,id);
                                break;
                            case Path::CATEGORY:
                                commands::select::selectSubCategory(db,pth,id);
                                break;
                            case Path::SUBCATEGORY:
                                commands::select::selectActivity(db,pth, id);
                                break;
                            case Path::ACTIVITY:
                                commands::select::selectTask(db,pth, id);
                                break;
                            case Path::TASK:
                                commands::select::selectSubTask(db,pth, id);
                                break;
                        }
                    }
                }
                if ((command == "ad") || (command == "add")){
                    std::string name, number, startDate, endDate, email, password, firstname, lastname;
                    int managerId;
                    double plannedHours;

                    switch(pth.current_type()){
                        case Path::ROOT:
                            std::cout << "Enter project name: ";
                            std::getline(std::cin, name);
                            std::cout << "Enter project number: ";
                            std::getline(std::cin, number);
                            std::cout << "Enter project manager ID: ";
                            std::cin >> managerId;
                            std::cin.ignore();
                            commands::add::addProject(db, name, number, managerId);
                            break;
                        case Path::EMPLOYEES:
                            std::cout << "Enter first name: ";
                            std::getline(std::cin, firstname);
                            std::cout << "Enter last name: ";
                            std::getline(std::cin, lastname);
                            std::cout << "Enter email: ";
                            std::getline(std::cin, email);
                            std::cout << "Enter password: ";
                            std::getline(std::cin, password);
                            commands::employee::addEmployee(db, firstname, lastname, email, password);
                            break;
                        case Path::PROJECT:
                            std::cout << "Enter category name: ";
                            std::getline(std::cin, name);
                            commands::add::addCategory(db, pth.current_id(), name);
                            break;
                        case Path::CATEGORY:
                            std::cout << "Enter subcategory name: ";
                            std::getline(std::cin, name);
                            commands::add::addSubcategory(db, pth.current_id(), name);
                            break;
                        case Path::SUBCATEGORY:
                            std::cout << "Enter activity name: ";
                            std::getline(std::cin, name);
                            std::cout << "Enter manager ID: ";
                            std::cin >> managerId;
                            std::cin.ignore();
                            std::cout << "Enter planned start date (YYYY-MM-DD): ";
                            std::getline(std::cin, startDate);
                            std::cout << "Enter planned end date (YYYY-MM-DD): ";
                            std::getline(std::cin, endDate);
                            std::cout << "Enter planned hours: ";
                            std::cin >> plannedHours;
                            std::cin.ignore();
                            commands::add::addActivity(db, name, pth.current_id(), managerId, startDate, endDate, plannedHours);
                            break;
                        case Path::ACTIVITY:
                            std::cout << "Enter task name: ";
                            std::getline(std::cin, name);
                            commands::add::addTask(db, pth.current_id(), name, 0);
                            break;
                        case Path::TASK: {
                            std::cout << "Enter subtask name: ";
                            std::getline(std::cin, name);
                            // Query the activity ID for the current task
                            auto taskStmt = db.prepareStatement("SELECT activityid FROM ActivityTasks WHERE taskid = ?");
                            taskStmt->setInt(1, pth.current_id());
                            auto taskRes = taskStmt->executeQuery();
                            if (taskRes->next()) {
                                int actId = taskRes->getInt("activityid");
                                commands::add::addTask(db, actId, name, pth.current_id());
                            } else {
                                std::cerr << "Error: Could not find activity for current task." << std::endl;
                            }
                            break;
                        }
                    }
                }
                if ((command == "rm") || (command == "remove")){
                    if (id == 0) {
                        std::cerr << "Error: Please provide a valid ID to remove." << std::endl;
                        continue;
                    }

                    // Ask for confirmation before destructive operation
                    std::string confirmMsg = "Are you sure you want to remove item with ID " + std::to_string(id) + "?";
                    if (!utils::confirmAction(confirmMsg)) {
                        std::cout << "Operation cancelled." << std::endl;
                        continue;
                    }

                    switch(pth.current_type()){
                        case Path::ROOT:
                            commands::remove::removeProject(db, id);
                            break;
                        case Path::EMPLOYEES:
                            commands::employee::removeEmployee(db, id);
                            break;
                        case Path::PROJECT:
                            commands::remove::removeCategory(db, id);
                            break;
                        case Path::CATEGORY:
                            commands::remove::removeSubCategory(db, id);
                            break;
                        case Path::SUBCATEGORY:
                            commands::remove::removeActivity(db, id);
                            break;
                        case Path::ACTIVITY:
                        case Path::TASK:
                            commands::remove::removeTask(db, id);
                            break;
                    }
                }
            }        
            //utils::printUsage();
            return 0;
        }

        std::string command = argv[1];
            
        // Route commands to appropriate handlers
        /*
        if (command == "import-activities") {
            if (argc < 4) {
                std::cerr << "Usage: osoem_admin import-activities <csv_file> <project_id>" << std::endl;
                return 1;
            }
            std::string csvFile = argv[2];
            int projectId = utils::toInt(argv[3]);
            commands::activities::importActivities(db, csvFile, projectId);

        } else if (command == "list-activities") {
            int projectId = (argc > 2) ? utils::toInt(argv[2]) : -1;
            commands::activities::listActivities(db, projectId);

        } else if (command == "add-activity") {
            if (argc < 8) {
                std::cerr << "Usage: osoem_admin add-activity <name> <subcategory_id> <manager_id> <start> <end> <hours>" << std::endl;
                return 1;
            }
            std::string name = argv[2];
            int subcategoryId = utils::toInt(argv[3]);
            int managerId = utils::toInt(argv[4]);
            std::string startDate = argv[5];
            std::string endDate = argv[6];
            double hours = utils::toDouble(argv[7]);
            commands::activities::addActivity(db, name, subcategoryId, managerId, startDate, endDate, hours);

        } else if (command == "remove-activity") {
            if (argc < 3) {
                std::cerr << "Usage: osoem_admin remove-activity <activity_id>" << std::endl;
                return 1;
            }
            int activityId = utils::toInt(argv[2]);
            commands::activities::removeActivity(db, activityId);

        } else if (command == "add-task") {
            if (argc < 4) {
                std::cerr << "Usage: osoem_admin add-task <activity_id> <task_name> [parent_task_id]" << std::endl;
                return 1;
            }
            int activityId = utils::toInt(argv[2]);
            std::string taskName = argv[3];
            int parentTaskId = (argc > 4) ? utils::toInt(argv[4]) : -1;
            commands::tasks::addTask(db, activityId, taskName, parentTaskId);

        } else if (command == "list-tasks") {
            if (argc < 3) {
                std::cerr << "Usage: osoem_admin list-tasks <activity_id>" << std::endl;
                return 1;
            }
            int activityId = utils::toInt(argv[2]);
            commands::tasks::listTasks(db, activityId);

        } else if (command == "create-artefact-type") {
            if (argc < 5) {
                std::cerr << "Usage: osoem_admin create-artefact-type <project_id> <name> <description>" << std::endl;
                return 1;
            }
            int projectId = utils::toInt(argv[2]);
            std::string name = argv[3];
            std::string description = argv[4];
            commands::artefacts::createArtefactType(db, projectId, name, description);

        } else if (command == "import-artefacts") {
            if (argc < 4) {
                std::cerr << "Usage: osoem_admin import-artefacts <csv_file> <artefact_type_id>" << std::endl;
                return 1;
            }
            std::string csvFile = argv[2];
            int artefactTypeId = utils::toInt(argv[3]);
            commands::artefacts::importArtefacts(db, csvFile, artefactTypeId);

        } else if (command == "list-artefact-types") {
            int projectId = (argc > 2) ? utils::toInt(argv[2]) : -1;
            commands::artefacts::listArtefactTypes(db, projectId);

        } else if (command == "list-artefacts") {
            if (argc < 3) {
                std::cerr << "Usage: osoem_admin list-artefacts <artefact_type_id>" << std::endl;
                return 1;
            }
            int artefactTypeId = utils::toInt(argv[2]);
            commands::artefacts::listArtefacts(db, artefactTypeId);

        } else if (command == "add-employee") {
            if (argc < 6) {
                std::cerr << "Usage: osoem_admin add-employee <firstname> <lastname> <email> <password>" << std::endl;
                return 1;
            }
            std::string firstName = argv[2];
            std::string lastName = argv[3];
            std::string email = argv[4];
            std::string password = argv[5];
            commands::employees::addEmployee(db, firstName, lastName, email, password);

        } else if (command == "remove-employee") {
            if (argc < 3) {
                std::cerr << "Usage: osoem_admin remove-employee <employee_id>" << std::endl;
                return 1;
            }
            int employeeId = utils::toInt(argv[2]);
            commands::employees::removeEmployee(db, employeeId);

        } else if (command == "list-employees") {
            commands::employees::listEmployees(db);

        } else if (command == "add-milestone") {
            if (argc < 4) {
                std::cerr << "Usage: osoem_admin add-milestone <project_id> <name>" << std::endl;
                return 1;
            }
            int projectId = utils::toInt(argv[2]);
            std::string name = argv[3];
            commands::milestones::addMilestone(db, projectId, name);

        } else if (command == "add-milestone-step") {
            if (argc < 5) {
                std::cerr << "Usage: osoem_admin add-milestone-step <milestone_id> <step_name> <progress_ratio>" << std::endl;
                return 1;
            }
            int milestoneId = utils::toInt(argv[2]);
            std::string stepName = argv[3];
            double ratio = utils::toDouble(argv[4]);
            commands::milestones::addMilestoneStep(db, milestoneId, stepName, ratio);

        } else if (command == "remove-milestone") {
            if (argc < 3) {
                std::cerr << "Usage: osoem_admin remove-milestone <milestone_id>" << std::endl;
                return 1;
            }
            int milestoneId = utils::toInt(argv[2]);
            commands::milestones::removeMilestone(db, milestoneId);

        } else if (command == "list-milestones") {
            int projectId = (argc > 2) ? utils::toInt(argv[2]) : -1;
            commands::milestones::listMilestones(db, projectId);

        } else if (command == "list-milestone-steps") {
            if (argc < 3) {
                std::cerr << "Usage: osoem_admin list-milestone-steps <milestone_id>" << std::endl;
                return 1;
            }
            int milestoneId = utils::toInt(argv[2]);
            commands::milestones::listMilestoneSteps(db, milestoneId);

        } else if (command == "link-artefact") {
            if (argc < 5) {
                std::cerr << "Usage: osoem_admin link-artefact <artefact_id> <activity_id> <ratio>" << std::endl;
                return 1;
            }
            int artefactId = utils::toInt(argv[2]);
            int activityId = utils::toInt(argv[3]);
            double ratio = utils::toDouble(argv[4]);
            commands::links::linkArtefactToActivity(db, artefactId, activityId, ratio);

        } else if (command == "link-milestone") {
            if (argc < 4) {
                std::cerr << "Usage: osoem_admin link-milestone <milestone_step_id> <artefact_link_id>" << std::endl;
                return 1;
            }
            int milestoneStepId = utils::toInt(argv[2]);
            int artefactLinkId = utils::toInt(argv[3]);
            commands::links::linkMilestoneToArtefact(db, milestoneStepId, artefactLinkId);

        } else if (command == "list-artefact-links") {
            int activityId = (argc > 2) ? utils::toInt(argv[2]) : -1;
            commands::links::listArtefactLinks(db, activityId);

        } else if (command == "list-milestone-links") {
            int milestoneId = (argc > 2) ? utils::toInt(argv[2]) : -1;
            commands::links::listMilestoneLinks(db, milestoneId);

        } else if (command == "help" || command == "--help" || command == "-h") {
            utils::printUsage();

        } else {
            std::cerr << "Unknown command: " << command << std::endl;
            utils::printUsage();
            return 1;
        }
        */
        return 0;

    } catch (sql::SQLException& e) {
        std::cerr << "\nDatabase error occurred:" << std::endl;
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (std::exception& e) {
        std::cerr << "\nError: " << e.what() << std::endl;
        return 1;
    }
}
