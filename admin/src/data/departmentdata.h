#ifndef DEPARTMENTDATA_H
#define DEPARTMENTDATA_H

#include <string>
#include <vector>
#include <map>
#include "database.h"
#include "datarecord.h"

namespace data {

class DepartmentData : public DataRecord {
public:
    // Constructor to manually create class
    DepartmentData(Database& db);

    // Constructor to create class using a vector of maps
    DepartmentData(Database& db, std::vector<std::map<std::string,std::string>> rows, bool ignoreDuplicates = false);

    // Constructor to create class from database
    DepartmentData( int departmentId, Database* db_ptr);

    // Setter function
    void setValue(std::string fieldName, std::string fieldValue);

    // Getter functions
    std::string getName() { return records[index].name; }

protected:
    std::size_t recordCount() const override { return records.size(); }

private:
    struct Record {
        int id = 0;
        std::string name;
    };
    std::vector<Record> records;
    Database& db;

    void validate();
    void validateRecord(Record& record);
};

} // namespace data

#endif // DEPARTMENTDATA_H
