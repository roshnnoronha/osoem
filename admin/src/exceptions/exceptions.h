#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <string>
#include <vector>

class DataError : public std::runtime_error {
public:
    explicit DataError(const std::string& message)
        : std::runtime_error(message) {}
};

class ValidationError : public DataError {
public:
    ValidationError()
        : DataError("Validation error") {}
    explicit ValidationError(const std::string& detail)
        : DataError( detail) {}
};

class CsvParseError : public DataError {
public:
    explicit CsvParseError(const std::string& message)
        : DataError(message) {}
    CsvParseError(const std::string& message, int lineNumber)
        : DataError(message + " on line #" + std::to_string(lineNumber)) {}
};

class InvalidFieldError : public DataError {
public:
    explicit InvalidFieldError(const std::string& message)
        : DataError(message) {}
};

class UserCancelledError : public DataError {
public:
    UserCancelledError()
        : DataError("Operation cancelled.") {}
};

class NavigationError : public DataError {
public:
    explicit NavigationError(const std::string& detail)
        : DataError(detail) {}
};

class EntityNotFoundError : public DataError {
public:
    EntityNotFoundError(const std::string& entityType, int id)
        : DataError(entityType + " with ID " + std::to_string(id) + " not found.") {}
    EntityNotFoundError(const std::string& entityType, const std::string& name)
        : DataError(entityType + " not found with name '" + name + "'.") {}
};

class DatabaseError : public DataError {
public:
    explicit DatabaseError(const std::string& message)
        : DataError(message) {}
    DatabaseError(const std::string& operation, const std::string& sqlError)
        : DataError("Database error during " + operation + ": " + sqlError) {}
};

class FileIOError : public DataError {
public:
    explicit FileIOError(const std::string& message)
        : DataError(message) {}
};

class PermissionError : public DataError {
public:
    PermissionError()
        : DataError("Permission denied. Admin access required.") {}
    explicit PermissionError(const std::string& detail)
        : DataError("Permission denied. " + detail) {}
};

class ConstraintViolationError : public DataError {
public:
    explicit ConstraintViolationError(const std::string& message)
        : DataError(message) {}
};

class CommandError : public DataError {
public:
    CommandError()
        : DataError("Unknown command.") {}
    explicit CommandError(const std::string& command)
        : DataError("Unknown command: " + command) {}
};

class DuplicateEntryError : public DataError {
public:
    DuplicateEntryError(const std::string& entityType, const std::string& name)
        : DataError("Duplicate " + entityType + " found: '" + name + "'.") {}
    explicit DuplicateEntryError(int count)
        : DataError(std::to_string(count) + " duplicate entr" + (count > 1 ? "ies" : "y") + " found.") {}
    DuplicateEntryError(int count, const std::string& entityType, const std::vector<std::string>& duplicateNames)
        : DataError(buildMessage(count, entityType, duplicateNames)) {}
private:
    static std::string buildMessage(int count, const std::string& entityType, const std::vector<std::string>& duplicateNames) {
        std::string msg = "Duplicate " + entityType + "(s) found: ";
        for (size_t i = 0; i < duplicateNames.size(); ++i) {
            if (i > 0) msg += ", ";
            msg += "'" + duplicateNames[i] + "'";
        }
        return msg;
    }
};

#endif // EXCEPTIONS_H
