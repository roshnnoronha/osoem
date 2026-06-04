#include "database.h"
#include "db_config.h"
#include "../exceptions/exceptions.h"
#include <iostream>
#include <sstream>

namespace data {

Database::Database() : driver(nullptr), conn(nullptr) {
    try {
        driver = sql::mysql::get_mysql_driver_instance();
        std::stringstream url;
        url << "tcp://" << config::DB_HOST << ":" << config::DB_PORT;
        conn.reset(driver->connect(url.str(), config::DB_USER, config::DB_PASS));
        conn->setSchema(config::DB_NAME);
        std::cout << "Database connection established.\n";
    } catch (sql::SQLException& e) {
        throw DatabaseError("connection", e.what());
    }
}

Database::~Database() {
    if (conn) conn->close();
}

sql::Connection* Database::getConnection() {
    return conn.get();
}

std::unique_ptr<sql::ResultSet> Database::executeQuery(const std::string& query) {
    try {
        std::unique_ptr<sql::Statement> stmt(conn->createStatement());
        return std::unique_ptr<sql::ResultSet>(stmt->executeQuery(query));
    } catch (sql::SQLException& e) {
        throw DatabaseError("query", e.what());
    }
}

int Database::executeUpdate(const std::string& query) {
    try {
        std::unique_ptr<sql::Statement> stmt(conn->createStatement());
        return stmt->executeUpdate(query);
    } catch (sql::SQLException& e) {
        throw DatabaseError("update", e.what());
    }
}

std::unique_ptr<sql::PreparedStatement> Database::prepareStatement(const std::string& query) {
    try {
        return std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(query));
    } catch (sql::SQLException& e) {
        throw DatabaseError("prepare statement", e.what());
    }
}

bool Database::isConnected() {
    return conn && !conn->isClosed();
}

} // namespace data
