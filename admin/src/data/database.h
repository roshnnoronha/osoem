#ifndef DATABASE_H
#define DATABASE_H

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/exception.h>
#include <memory>
#include <string>

namespace data {

class Database {
private:
    sql::mysql::MySQL_Driver* driver;
    std::unique_ptr<sql::Connection> conn;

public:
    Database();
    ~Database();

    // Get the connection object
    sql::Connection* getConnection();

    // Execute a query and return result set
    std::unique_ptr<sql::ResultSet> executeQuery(const std::string& query);

    // Execute an update/insert/delete query
    int executeUpdate(const std::string& query);

    // Create a prepared statement
    std::unique_ptr<sql::PreparedStatement> prepareStatement(const std::string& query);

    // Begin transaction
    void beginTransaction();

    // Commit transaction
    void commit();

    // Rollback transaction
    void rollback();

    // Check if connected
    bool isConnected();

    // Get the last auto-generated insert ID
    int getLastInsertId();
};

} // namespace data

#endif // DATABASE_H
