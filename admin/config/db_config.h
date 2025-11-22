#ifndef DB_CONFIG_H
#define DB_CONFIG_H

#include <string>

namespace config {
    // Database connection parameters
    const std::string DB_HOST = "localhost";
    const std::string DB_USER = "roshn";
    const std::string DB_PASS = "P@ssword";
    const std::string DB_NAME = "trial";
    const int DB_PORT = 3306;
}

#endif // DB_CONFIG_H
