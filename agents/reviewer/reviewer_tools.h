#ifndef REVIEWER_TOOLS_H
#define REVIEWER_TOOLS_H

#include "../client/openai_client.h"
#include "../data/database.h"
#include "spec_indexer.h"

#include <string>
#include <vector>

namespace reviewer {

class ReviewerTools {
public:
    ReviewerTools(data::Database& db, client::OpenAIClient& client, const std::string& specsDir);
    std::vector<client::Tool> getTools();
    std::string callTool(const client::ToolCall& toolCall);
    void setWorkingActivity(int activityId);
    std::vector<std::pair<int, std::string>> getActivityList();

private:
    data::Database& db_;
    std::string specsDir_;
    int workingActivityID_ = 0;
    SpecIndexer specIndexer_;

    std::string getWorkingActivity();
    std::string getArtefactsInActivity(int activityId);
    std::string readDocument(int artefactId);
    std::string getSpecList();
    std::string readSpec(const std::string& filename);
    std::string addReviewComment(int activityId, const std::string& title,
        const std::string& location, const std::string& issue,
        const std::string& specReference, const std::string& severity);

    int findOrCreateCommentArtefactType(int projectId);
    int findOrCreateField(int artefactTypeId, const std::string& fieldTitle);
};

} // namespace reviewer

#endif // REVIEWER_TOOLS_H
