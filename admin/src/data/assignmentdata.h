#ifndef ASSIGNMENTDATA_H
#define ASSIGNMENTDATA_H

#include <string>
#include <vector>
#include <map>

#include "database.h"
#include "datarecord.h"

namespace data {

class AssignmentData : public DataRecord {
public:
    // Constructor to manually create class
    AssignmentData(Database& db, int taskId, int projectId);

    // Constructor to create class using a vector of maps
    AssignmentData(Database& db, std::vector<std::map<std::string,std::string>> rows, int taskId, bool ignoreDuplicates = false);
    
    // Constructor to create class from database
    AssignmentData( int assignmentId, Database* db_ptr);

    // Setter function
    void setValue(std::string fieldName, std::string fieldValue);

    // Getter functions
    int getTaskId() { return records[index].taskId; }
    int getUserId() { return records[index].userId; }
    std::string getAssignedDate() { return records[index].assignedDate; }
    std::string getCloseDate() { return records[index].closeDate; }

protected:
    std::size_t recordCount() const override { return records.size(); }

private:
    struct Record {
        int id = 0;
        int taskId;
        int userId;
        std::string assignedDate;
        std::string closeDate;
    };
    std::vector<Record> records;
    Database& db;

    void validate();
    void validateRecord(Record& record);
};

} // namespace data

#endif // ASSIGNMENTDATA_H
