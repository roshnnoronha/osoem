#include "database.h"
#include "db_config.h"
#include "../exceptions/exceptions.h"
#include <iostream>
#include <sstream>

namespace data {

Database::Database() : driver(nullptr), conn(nullptr) {
    try {
        driver = sql::mysql::get_mysql_driver_instance();
        std::stringstream connectionString;
        connectionString << "tcp://" << config::DB_HOST << ":" << config::DB_PORT;

        conn.reset(driver->connect(connectionString.str(), config::DB_USER, config::DB_PASS));
        conn->setSchema(config::DB_NAME);

        std::cout << "Database connection established successfully." << std::endl;
    } catch (sql::SQLException& e) {
        std::cerr << "Database connection error: " << e.what() << std::endl;
        std::cerr << "MySQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
        throw DatabaseError("connection", e.what());
    }
}

Database::~Database() {
    if (conn) {
        conn->close();
    }
}

sql::Connection* Database::getConnection() {
    return conn.get();
}

std::unique_ptr<sql::ResultSet> Database::executeQuery(const std::string& query) {
    try {
        std::unique_ptr<sql::Statement> stmt(conn->createStatement());
        return std::unique_ptr<sql::ResultSet>(stmt->executeQuery(query));
    } catch (sql::SQLException& e) {
        std::cerr << "Query error: " << e.what() << std::endl;
        std::cerr << "Query: " << query << std::endl;
        throw DatabaseError("query", e.what());
    }
}

int Database::executeUpdate(const std::string& query) {
    try {
        std::unique_ptr<sql::Statement> stmt(conn->createStatement());
        return stmt->executeUpdate(query);
        // Function to remove a task given a task id.
    } catch (sql::SQLException& e) {
        std::cerr << "Update error: " << e.what() << std::endl;
        std::cerr << "Query: " << query << std::endl;
        throw DatabaseError("update", e.what());
    }
}

std::unique_ptr<sql::PreparedStatement> Database::prepareStatement(const std::string& query) {
    try {
        return std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(query));
    } catch (sql::SQLException& e) {
        std::cerr << "Prepare statement error: " << e.what() << std::endl;
        throw DatabaseError("prepare statement", e.what());
    }
}

void Database::beginTransaction() {
    conn->setAutoCommit(false);
}

void Database::commit() {
    conn->commit();
    conn->setAutoCommit(true);
}

void Database::rollback() {
    conn->rollback();
    conn->setAutoCommit(true);
}

bool Database::isConnected() {
    return conn && !conn->isClosed();
}

int Database::getLastInsertId() {
    auto res = executeQuery("SELECT LAST_INSERT_ID()");
    if (res->next()) {
        return res->getInt(1);
    }
    return 0;
}

} // namespace data
