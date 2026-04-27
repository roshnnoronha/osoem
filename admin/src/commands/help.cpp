#include "help.h"
#include <iostream>

namespace commands {

void help(const std::string& topic) {
    if (topic.empty()) {
        std::cout <<
            "osoem admin - Database administration tool for engineering project management\n"
            "\n"
            "Navigate the project data hierarchy using the commands below.\n"
            "Use 'help <command>' for detailed usage of a specific command.\n"
            "\n"
            "Commands:\n"
            "  list   (ls)      List items at the current location\n"
            "  select (cd, sl)  Navigate to an item or folder\n"
            "  add    (ad)      Add a new item\n"
            "  remove (rm)      Remove an item\n"
            "  set    (st)      Edit a field value on an item\n"
            "  import (im)      Import items from a CSV file\n"
            "  export (ex)      Export items to a CSV file\n"
            "  help   (h)       Show this help\n"
            "  exit   (quit)    Exit the program\n";
        return;
    }

    if (topic == "list" || topic == "ls") {
        std::cout <<
            "list (ls)\n"
            "\n"
            "  List items at the current location.\n"
            "\n"
            "  Syntax:  list [-a]\n"
            "           ls   [-a]\n"
            "\n"
            "  Flags:\n"
            "    -a    Show all fields in a full table (default shows ID and name only)\n"
            "\n"
            "  Examples:\n"
            "    ls        List items (ID + name)\n"
            "    list -a   List items with all fields\n";

    } else if (topic == "select" || topic == "cd" || topic == "sl") {
        std::cout <<
            "select (cd, sl)\n"
            "\n"
            "  Navigate to an item or folder in the hierarchy.\n"
            "\n"
            "  Syntax:  select <name|id|..|~|path>\n"
            "           cd     <name|id|..|~|path>\n"
            "           sl     <name|id|..|~|path>\n"
            "\n"
            "  Special arguments:\n"
            "    ..        Move up one level\n"
            "    ~         Return to root\n"
            "    ~/a/b/c   Navigate a multi-level path from root\n"
            "\n"
            "  Notes:\n"
            "    - Item names containing spaces must be quoted: cd \"My Project\"\n"
            "    - Names beginning with '.' are reserved and not allowed\n"
            "    - '/' is a reserved separator character\n"
            "\n"
            "  Examples:\n"
            "    cd projects          Navigate into the projects folder\n"
            "    cd \"My Project\"      Navigate into an item by name\n"
            "    cd ..                Go up one level\n"
            "    cd ~                 Return to root\n"
            "    cd ~/projects        Navigate to projects from root\n";

    } else if (topic == "add" || topic == "ad") {
        std::cout <<
            "add (ad)\n"
            "\n"
            "  Add a new item at the current location.\n"
            "\n"
            "  Syntax:  add\n"
            "           add <field1>,<field2>,...\n"
            "           ad  <field1>,<field2>,...\n"
            "\n"
            "  Usage:\n"
            "    - With no arguments, the program prompts for each field interactively.\n"
            "    - With CSV arguments, all fields are supplied in a single line (used in\n"
            "      source files — interactive mode is not available from source files).\n"
            "\n"
            "  Examples:\n"
            "    add                           Interactive add\n"
            "    ad \"Project Alpha\",2026-01-01  Add with CSV values\n";

    } else if (topic == "remove" || topic == "rm") {
        std::cout <<
            "remove (rm)\n"
            "\n"
            "  Remove an item at the current location.\n"
            "\n"
            "  Syntax:  remove <name|id>\n"
            "           rm     <name|id>\n"
            "\n"
            "  Notes:\n"
            "    - Prompts for confirmation when run interactively.\n"
            "    - Confirmation is suppressed when running from a source file.\n"
            "    - Removing an item may cascade-delete dependent child records.\n"
            "\n"
            "  Examples:\n"
            "    rm \"Task 1\"   Remove item named 'Task 1'\n"
            "    rm 42         Remove item with ID 42\n";

    } else if (topic == "set" || topic == "st") {
        std::cout <<
            "set (st)\n"
            "\n"
            "  Edit the value of a field on an item at the current location.\n"
            "\n"
            "  Syntax:  set <id|name> <fieldname>,<newvalue>\n"
            "           st  <id|name> <fieldname>,<newvalue>\n"
            "\n"
            "  Notes:\n"
            "    - The first argument identifies the item (by ID or name).\n"
            "    - The second argument is a comma-separated pair: field name and new value.\n"
            "    - Some fields (e.g. parent IDs) cannot be edited via set.\n"
            "\n"
            "  Examples:\n"
            "    set 5 activityname,\"New Name\"   Rename activity with ID 5\n"
            "    st \"Task 1\" startdate,2026-06-01 Set start date on a task\n";

    } else if (topic == "import" || topic == "im") {
        std::cout <<
            "import (im)\n"
            "\n"
            "  Import items from a CSV file into the current location.\n"
            "\n"
            "  Syntax:  import <file>\n"
            "           import -r <file>   Recursive import\n"
            "           import -i <file>   Ignore duplicate entries\n"
            "           im     <file>\n"
            "\n"
            "  Flags:\n"
            "    -r    Recursively import a hierarchical CSV (e.g. activities with\n"
            "          sub-tasks, assignments, and hours in one file)\n"
            "    -i    Skip rows that would create duplicate records instead of erroring\n"
            "\n"
            "  Notes:\n"
            "    - File paths may be absolute or relative to the source file when\n"
            "      running in batch mode.\n"
            "    - Press Tab to autocomplete file paths during interactive use.\n"
            "\n"
            "  Examples:\n"
            "    im data.csv            Import from data.csv\n"
            "    import -r activity.csv Recursively import an activity hierarchy\n"
            "    import -i employees.csv Import, skipping duplicates\n";

    } else if (topic == "export" || topic == "ex") {
        std::cout <<
            "export (ex)\n"
            "\n"
            "  Export items at the current location to a CSV file.\n"
            "\n"
            "  Syntax:  export <file>\n"
            "           ex     <file>\n"
            "\n"
            "  Notes:\n"
            "    - Exports the items listed at the current hierarchy level.\n"
            "    - File paths may be absolute or relative to the current working directory.\n"
            "\n"
            "  Examples:\n"
            "    ex output.csv          Export to output.csv\n"
            "    export /tmp/data.csv   Export to an absolute path\n";

    } else if (topic == "exit" || topic == "quit") {
        std::cout <<
            "exit (quit)\n"
            "\n"
            "  Exit the osoem admin tool.\n"
            "\n"
            "  Syntax:  exit\n"
            "           quit\n";

    } else if (topic == "help" || topic == "h") {
        std::cout <<
            "help (h)\n"
            "\n"
            "  Display program usage or detailed help for a command.\n"
            "\n"
            "  Syntax:  help\n"
            "           help <command>\n"
            "           h    <command>\n"
            "\n"
            "  Examples:\n"
            "    help           Show all commands\n"
            "    help import    Show import command details\n"
            "    h list         Show list command details\n";

    } else {
        std::cout << "Unknown command: '" << topic << "'. Run 'help' to see all commands.\n";
    }
}

} // namespace commands
