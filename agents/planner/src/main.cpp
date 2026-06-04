#include <iostream>
#include "data/database.h"
#include "planner/openai_client.h"
#include "planner/planner.h"
#include "api_config.h"

int main() {
    try {
        data::Database db;

        planner::OpenAIClient client(
            config::OPENAI_API_KEY,
            config::OPENAI_MODEL,
            config::OPENAI_API_URL
        );

        planner::Planner agent(client, db);
        agent.run();

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
