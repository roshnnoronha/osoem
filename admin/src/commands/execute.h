#ifndef EXECUTE_H
#define EXECUTE_H

#include "list.h"
#include "select.h"
#include "add.h"
#include "remove.h"
#include "importcsv.h"
#include "exportcsv.h"
#include "set.h"
#include "help.h"
#include "../data/database.h"
#include "../data/path.h"

#include <filesystem>
#include <string>
#include <vector>

using namespace data;

namespace commands {

// Execute a command line. Returns 1 if command is `exit`.
int execute (Database& db, Path& pth, std::string input, bool isAdmin = 0, bool fromSourceFile = false, std::string sourceDir = "");

// Execute a batch of commands in a source file `filename`
void batchExecute (Database& db, Path& pth, std::string filename);

// Parse input string into tokens, handling quoted strings with spaces
std::vector<std::string> parseInput(const std::string& input); 

} //namespace commands
#endif //EXECUTE_H
