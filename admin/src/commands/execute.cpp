#include "execute.h"
#include "../exceptions/exceptions.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>


namespace commands {

// Execute a command line
int execute (Database& db, Path& pth, std::string input, bool isAdmin, bool fromSourceFile, std::string sourceDir) {
    std::vector<std::string> tokens;
    std::string command;
    std::string param;
    std::string param1;

    tokens = parseInput(input);
    if (tokens.empty()) return 0;

    command = tokens[0];
    param = (tokens.size() > 1) ? tokens[1] : "";
    param1 = (tokens.size() > 2) ? tokens[2] : "";
    // Resolve a file path relative to the source file's directory when running in batch mode
    auto resolvePath = [&](const std::string& f) -> std::string {
        if (!sourceDir.empty() && !std::filesystem::path(f).is_absolute())
            return (std::filesystem::path(sourceDir) / f).string();
        return f;
    };

    if ((command == "exit") || (command == "quit")) {
        return 1;
    } else if ((command == "ls") || (command == "list")){
        bool showAll = (param == "-a");
        list(db, pth, showAll);
    } else if ((command == "cd") || (command == "sl") || (command == "select")){
        select(db, pth, param);
    } else if ((command == "ad") || (command == "add")){
        if (!isAdmin) {
            throw PermissionError();
        }
        if (param.empty()) {
            if (fromSourceFile) {
                throw PermissionError("Manual add is not allowed when running from a source file. Use: ad <field1>,<field2>,...");
            }
            add(db, pth);
        } else {
            add(db, pth, param);
        }
    } else if ((command == "rm") || (command == "remove")){
        if (!isAdmin) {
            throw PermissionError();
        }
        remove(db, pth, param, fromSourceFile);
    } else if ((command == "st") || (command == "set")){
        if (!isAdmin) {
            throw PermissionError();
        }
        set(db, pth, param, param1);
    } else if ((command == "im") || (command == "import")) {
        if (!isAdmin) {
            throw PermissionError();
        }
        bool recursive = (param == "-r");
        bool ignoreDuplicates = (param == "-i");
        std::string importFile = (recursive || ignoreDuplicates) ? param1 : param;
        if (recursive) {
            importcsvRecursive(db, pth, resolvePath(importFile));
        } else {
            importcsv(db, pth, resolvePath(importFile), ignoreDuplicates);
        }
    } else if ((command == "ex") || (command == "export")) {
        exportcsv(db, pth, resolvePath(param));
    } else if ((command == "h") || (command == "help")) {
        help(param);
    } else {
        throw CommandError();
    }
    return 0;
}

// Execute a batch of commands in a source file `filename`
void batchExecute (Database& db, Path& pth, std::string filename) {
    // Open file
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw FileIOError("Could not open file '" + filename + "'");
    }

    // Determine the directory containing the source file so that relative paths
    // in `im` / `ex` commands resolve relative to the source file, not the CWD.
    std::string sourceDir = std::filesystem::path(filename).parent_path().string();

    std::string line;
    int lineNumber = 0;

    // Read and execute each line
    while (std::getline(file, line)) {
        lineNumber++;

        // Skip empty lines and comments (lines starting with #)
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::cout << "[osoem: "<<filename<<", Line " << lineNumber << "] "<< pth.to_string() << ">" << line << std::endl;
        try {
            execute (db, pth, line, 1, true, sourceDir);
        } catch (const DataError& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return;
        }

    }

    file.close();
    std::cout << "Finished executing commands from file." << std::endl;

}

// Parse input string into tokens, handling quoted strings with spaces
std::vector<std::string> parseInput(const std::string& input) {
    std::vector<std::string> tokens;
    std::string currentToken;
    bool inQuotes = false;
    char quoteChar = '\0';

    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];

        if (inQuotes) {
            // Check for escaped quote
            if (c == '\\' && i + 1 < input.length() && input[i + 1] == quoteChar) {
                currentToken += quoteChar;
                ++i; // Skip the escaped quote
            } else if (c == quoteChar) {
                // End of quoted string
                inQuotes = false;
                quoteChar = '\0';
            } else {
                currentToken += c;
            }
        } else {
            if (c == '"' || c == '\'') {
                // Start of quoted string
                inQuotes = true;
                quoteChar = c;
            } else if (c == ' ' || c == '\t') {
                // Whitespace - end current token if non-empty
                if (!currentToken.empty()) {
                    tokens.push_back(currentToken);
                    currentToken.clear();
                }
            } else {
                currentToken += c;
            }
        }
    }

    // Add the last token if non-empty
    if (!currentToken.empty()) {
        tokens.push_back(currentToken);
    }

    return tokens;
}

} //namespace commands
