#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "data/path.h"
#include "data/database.h"

using namespace data;

#include "commands/execute.h"
#include "utils/utils.h"
#include "utils/auth.h"
#include "utils/completion_provider.h"
#include "utils/readline_wrapper.h"
#include "exceptions/exceptions.h"

int main(int argc, char* argv[]) {

    // Parse command line arguments
    std::string userEmail;
    std::string sourceFile;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-u" && i + 1 < argc) {
            userEmail = argv[++i];
        } else if (arg[0] != '-') {
            sourceFile = arg;
        }
    }

    try {
        // Initialize database connection
        Database db;

        if (!db.isConnected()) {
            std::cerr << "Failed to connect to database." << std::endl;
            return 1;
        }

        // Initialize user context
        auth::UserContext userContext;

        // If -u flag provided, perform authentication
        if (!userEmail.empty()) {
            std::string password = auth::promptPassword("Password: ");

            if (!auth::authenticate(db, userEmail, password, userContext)) {
                std::cerr << "Authentication failed. Invalid email or password." << std::endl;
                return 1;
            }

            std::cout << "Logged in as: " << userContext.firstName << " "
                      << userContext.lastName << " (" << userContext.email << ")" << std::endl;
            std::cout << "Role: " << (userContext.isAdmin ? "Admin" : "User") << std::endl;
        }

        // Initialize path
        Path pth;

        // Check if we have a source file to execute (batch mode)
        if (!sourceFile.empty()) {
            commands::batchExecute(db, pth, sourceFile);
            return 0;
        }

        // Interactive mode (no source file argument)
        if (sourceFile.empty()) {
            std::string inp;

            // Initialize tab completion
            CompletionProvider completionProvider(db, pth);
            readline_wrapper::initialize(&completionProvider);

            while (true) {
                std::string prompt = "[osoem]" + pth.to_string() + "> ";
                inp = readline_wrapper::readLine(prompt);

                // Check for EOF (Ctrl+D)
                if (inp.empty() && std::cin.eof()) {
                    std::cout << std::endl;
                    break;
                }

                // Add non-empty lines to history
                if (!inp.empty()) {
                    readline_wrapper::addHistory(inp);
                }

                try {
                    if (commands::execute(db , pth, inp,userContext.isAdmin) == 1) break;
                } catch (const DataError& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                }
            }

            readline_wrapper::cleanup();
            return 0;
        }

        return 0;

    } catch (const DatabaseError& e) {
        std::cerr << "\nError: " << e.what() << std::endl;
        return 1;
    } catch (sql::SQLException& e) {
        std::cerr << "\nDatabase error: " << e.what() << std::endl;
        return 1;
    } catch (std::exception& e) {
        std::cerr << "\nError: " << e.what() << std::endl;
        return 1;
    }
}
