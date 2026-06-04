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
public:
    Database();
    ~Database();

    sql::Connection* getConnection();
    std::unique_ptr<sql::ResultSet> executeQuery(const std::string& query);
    int executeUpdate(const std::string& query);
    std::unique_ptr<sql::PreparedStatement> prepareStatement(const std::string& query);
    bool isConnected();

private:
    sql::mysql::MySQL_Driver* driver;
    std::unique_ptr<sql::Connection> conn;
};

} // namespace data

#endif // DATABASE_H
