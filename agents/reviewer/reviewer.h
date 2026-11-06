#ifndef REVIEWER_H
#define REVIEWER_H

#include "reviewer_tools.h"

#include <vector>
#include <string>

namespace reviewer {

class Reviewer {
public:
    Reviewer(client::OpenAIClient& client, data::Database& db, const std::string& specsDir);
    void run();

private:
    client::OpenAIClient& client_;
    std::vector<client::Message> messages_;
    ReviewerTools tools_;
    void selectWorkingActivity();
    std::string readInput(const std::string& prompt);
    std::string buildSystemPrompt();
};

} // namespace reviewer

#endif // REVIEWER_H
