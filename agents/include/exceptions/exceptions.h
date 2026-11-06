#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <string>

class DataError : public std::runtime_error {
public:
    explicit DataError(const std::string& message)
        : std::runtime_error(message) {}
};

class DatabaseError : public DataError {
public:
    explicit DatabaseError(const std::string& message)
        : DataError(message) {}
    DatabaseError(const std::string& operation, const std::string& sqlError)
        : DataError("Database error during " + operation + ": " + sqlError) {}
};

#endif // EXCEPTIONS_H
