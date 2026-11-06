#include "reviewer.h"
#include "../utils/utils.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <readline/readline.h>
#include <readline/history.h>

namespace {

std::vector<std::string> g_activityNames;

char* activityNameGenerator(const char* text, int state) {
    static size_t idx, len;
    if (state == 0) {
        idx = 0;
        len = strlen(text);
    }
    while (idx < g_activityNames.size()) {
        const auto& name = g_activityNames[idx++];
        if (strncasecmp(name.c_str(), text, len) == 0)
            return strdup(name.c_str());
    }
    return nullptr;
}

char** activityCompletion(const char* text, int /*start*/, int /*end*/) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, activityNameGenerator);
}

} // anonymous namespace

namespace reviewer {

Reviewer::Reviewer(client::OpenAIClient& client, data::Database& db, const std::string& specsDir)
    : client_(client), tools_(db, client, specsDir) {
    messages_.push_back({"system", buildSystemPrompt()});
}

std::string Reviewer::buildSystemPrompt() {
    std::ostringstream ss;

    ss << "You are the Osoem Document Reviewer, an AI assistant for engineering project management.\n"
       << "Your role is to review documents linked to a working activity against engineering specifications\n"
       << "and record non-compliance findings as Review Comment artefacts in the database.\n\n"

       << "## Working Activity\n"
       << "A working activity is set at startup. All review comments you create are linked to this activity.\n"
       << "Use get_working_activity to confirm which activity is active.\n\n"

       << "## Read Capabilities\n"
       << "- get_working_activity — show the current working activity and its project context.\n"
       << "- get_artefacts_in_activity — list all artefacts linked to the working activity, with their\n"
       << "  type, title, and data fields. Document artefacts will have a 'path' field with the file location.\n"
       << "- read_document(artefact_id) — read the file content of a document artefact.\n"
       << "- get_spec_list — list available specification files.\n"
       << "- search_specs(query) — semantically search all specification files for sections relevant\n"
       << "  to a query. Use this first to find specific topics without loading entire files.\n"
       << "- read_spec(filename) — read the complete content of a specification file. Use when you\n"
       << "  need full context or when search_specs does not return sufficient detail.\n\n"

       << "## Write Capabilities\n"
       << "- add_review_comment(title, location, issue, spec_reference, severity) — create a Review Comment\n"
       << "  artefact linked to the working activity. Severity must be Minor, Major, or Critical.\n\n"

       << "## Review Process\n"
       << "1. Use get_artefacts_in_activity to find document artefacts to review.\n"
       << "2. Use read_document to read each document's content.\n"
       << "3. Use search_specs to locate relevant specification sections. Use read_spec only when\n"
       << "   the full file context is required.\n"
       << "4. For each non-compliance found, use add_review_comment to record it.\n"
       << "5. Be precise about location (e.g. 'Section 3.2, Table 1') and cite the specific spec clause.\n\n"

       << "## Guidelines\n"
       << "- Only create comments for genuine non-compliances, not for items already satisfied.\n"
       << "- Be concise and technical in issue descriptions.\n"
       << "- Always cite the specific specification clause (e.g. 'AISC 360 Section J3.4') in spec_reference.\n"
       << "- Before recording comments, confirm the review scope with the user.\n";

    return ss.str();
}

void Reviewer::selectWorkingActivity() {
    auto activities = tools_.getActivityList();

    g_activityNames.clear();
    for (const auto& [id, name] : activities)
        g_activityNames.push_back(name);

    rl_attempted_completion_function = activityCompletion;

    while (true) {
        char* raw = readline("Select working activity (ID or name): ");
        if (!raw) {
            std::cout << "\nNo activity selected. Exiting.\n";
            exit(0);
        }
        std::string input(raw);
        free(raw);

        if (input.empty()) continue;

        bool matched = false;
        // Try numeric ID first
        try {
            size_t pos;
            int id = std::stoi(input, &pos);
            if (pos == input.size()) {
                for (const auto& [aid, aname] : activities) {
                    if (aid == id) {
                        tools_.setWorkingActivity(id);
                        std::cout << "Working activity set to: " << aname << "\n\n";
                        matched = true;
                        break;
                    }
                }
                if (!matched)
                    std::cout << "No activity with ID " << id << " found.\n";
            }
        } catch (...) {}

        // Try name match (case-insensitive)
        if (!matched) {
            std::string lower = utils::trim(input);
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            for (const auto& [aid, aname] : activities) {
                std::string anameLower = aname;
                std::transform(anameLower.begin(), anameLower.end(), anameLower.begin(), ::tolower);
                if (anameLower == lower) {
                    tools_.setWorkingActivity(aid);
                    std::cout << "Working activity set to: " << aname << "\n\n";
                    matched = true;
                    break;
                }
            }
            if (!matched)
                std::cout << "No activity matching \"" << input << "\" found.\n";
        }

        if (matched) break;
    }

    rl_attempted_completion_function = nullptr;
    g_activityNames.clear();
}

void Reviewer::run() {
    std::cout << "Osoem Reviewer Agent\n"
              << "====================\n"
              << "Review documents against specifications and record findings.\n"
              << "Commands: 'clear' resets the conversation, 'exit' or 'quit' to stop.\n\n";

    selectWorkingActivity();

    while (true) {
        std::string input = readInput("> ");
        if (input.empty()) continue;

        if (input == "exit" || input == "quit") {
            std::cout << "Goodbye.\n";
            break;
        }

        if (input == "clear") {
            messages_.erase(messages_.begin() + 1, messages_.end());
            std::cout << "Conversation cleared.\n\n";
            continue;
        }

        const size_t rollbackSize = messages_.size();
        messages_.push_back({"user", input});
        std::cout << "\n";
        try {
            while (true) {
                client::Response response = client_.chat(messages_, tools_.getTools());
                if (response.type == client::TOOL_CALL) {
                    messages_.push_back({"assistant", response.message, response.toolCalls});
                    for (const auto& tool : response.toolCalls) {
                        messages_.push_back({"tool", tools_.callTool(tool), {}, tool.id});
                    }
                } else {
                    messages_.push_back({"assistant", response.message});
                    std::cout << response.message << "\n\n";
                    break;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n\n";
            messages_.resize(rollbackSize);
        }
    }
}

std::string Reviewer::readInput(const std::string& prompt) {
    char* line = readline(prompt.c_str());
    if (!line) return "exit";
    std::string input(line);
    if (!input.empty()) add_history(line);
    free(line);
    return input;
}

} // namespace reviewer
