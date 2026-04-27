#ifndef TASKDATA_H
#define TASKDATA_H

#include <string>
#include <vector>
#include <map>

#include "database.h"
#include "datarecord.h"

namespace data {

class TaskData : public DataRecord {
public:
    // Constructor to manually create class with a given activity ID, and if it is a sub task a task ID
    TaskData(Database& db, int activityId, int taskId=0);

    // Constructor to create class using a vector of maps
    TaskData(Database& db, std::vector<std::map<std::string,std::string>> rows, int activityId, int taskId=0, bool ignoreDuplicates = false);

    // Constructor to create class from database
    TaskData( int taskId, Database* db_ptr);

    // Setter function
    void setValue(std::string fieldName, std::string fieldValue);

    // Getter functions
    std::string getName(){return records[index].name;}
    std::string getDescription(){return records[index].description;}
    int getActivityId(){return records[index].activityId;}
    int getParentTaskId(){return records[index].parentTaskId;}
    int getDepartmentId(){return records[index].departmentId;}

protected:
    std::size_t recordCount() const override { return records.size(); }

private:
    struct Record{
        int id = 0;
        std::string name;
        std::string description;
        int activityId;
        int parentTaskId;
        int departmentId;
    };
    std::vector<Record> records;
    Database& db;

    void validate();
    void validateRecord(Record& record);
};

} // namespace data

#endif // TASKDATA_H
