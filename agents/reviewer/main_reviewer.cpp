#include <iostream>
#include "data/database.h"
#include "client/openai_client.h"
#include "reviewer.h"
#include "api_config.h"

int main() {
    try {
        data::Database db;

        client::OpenAIClient client(
            config::OPENAI_API_KEY,
            config::OPENAI_MODEL,
            config::OPENAI_API_URL,
            config::OPENAI_EMBEDDING_MODEL,
            config::OPENAI_EMBEDDING_URL
        );

        reviewer::Reviewer agent(client, db, "specs");
        agent.run();

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
