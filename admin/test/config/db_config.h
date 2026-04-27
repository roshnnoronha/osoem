#ifndef DB_CONFIG_H
#define DB_CONFIG_H

#include <string>

namespace config {
    // Test database connection parameters — uses osoem_test, NOT the dev database
    const std::string DB_HOST = "localhost";
    const std::string DB_USER = "roshn";
    const std::string DB_PASS = "p@ssword";
    const std::string DB_NAME = "osoem_test";
    const int DB_PORT = 3306;
}

#endif // DB_CONFIG_H
