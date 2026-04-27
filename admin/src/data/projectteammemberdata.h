#ifndef PROJECTTEAMMEMBERDATA_H
#define PROJECTTEAMMEMBERDATA_H

#include <string>
#include <vector>
#include <map>

#include "database.h"
#include "datarecord.h"

namespace data {

class ProjectTeamMemberData : public DataRecord {
public:
    // Constructor to manually create class
    ProjectTeamMemberData(Database& db, int projectId);

    // Constructor to create class using a vector of maps
    ProjectTeamMemberData(Database& db, std::vector<std::map<std::string,std::string>> rows, int projectId, bool ignoreDuplicates = false);
    
    // Constructor to create class from database
    ProjectTeamMemberData( int teamMemberId, Database* db_ptr);

    // Setter function
    void setValue(std::string fieldName, std::string fieldValue);

    // Getter functions
    int getProjectId() { return records[index].projectId; }
    int getEmployeeId() { return records[index].employeeId; }
    int getRole() { return records[index].role; }

protected:
    std::size_t recordCount() const override { return records.size(); }

private:
    struct Record {
        int id = 0;
        int projectId;
        int employeeId;
        int role;
    };
    std::vector<Record> records;
    Database& db;

    void validate();
    void validateRecord(Record& record);
};

} // namespace data

#endif // PROJECTTEAMMEMBERDATA_H
