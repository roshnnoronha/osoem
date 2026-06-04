#ifndef PLANNER_H
#define PLANNER_H

#include "agent_tools.h"

#include <vector>
#include <string>

namespace planner {

class Planner {
public:
    Planner(OpenAIClient& client, data::Database& db);
    void run();

private:
    OpenAIClient& client_;
    std::vector<Message> messages_;
    AgentTools tools_;
    void selectWorkingProject();
    std::string readInput(const std::string& prompt);
    std::string buildSystemPrompt();
};

} // namespace planner

#endif // PLANNER_H
