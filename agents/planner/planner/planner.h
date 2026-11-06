#ifndef PLANNER_H
#define PLANNER_H

#include "planner_tools.h"

#include <vector>
#include <string>

namespace planner {

class Planner {
public:
    Planner(client::OpenAIClient& AIClient, data::Database& db);
    void run();

private:
    client::OpenAIClient& client_;
    std::vector<client::Message> messages_;
    PlannerTools tools_;
    void selectWorkingProject();
    std::string readInput(const std::string& prompt);
    std::string buildSystemPrompt();
};

} // namespace planner

#endif // PLANNER_H
