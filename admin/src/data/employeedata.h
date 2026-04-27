#ifndef EMPLOYEEDATA_H
#define EMPLOYEEDATA_H

#include <string>
#include <vector>
#include <map>

#include "database.h"
#include "datarecord.h"

namespace data {

class EmployeeData : public DataRecord {
public:
    // Constructor to manually create class
    EmployeeData(Database& db, int departmentId);

    // Constructor to create class using a vector of maps
    EmployeeData(Database& db, std::vector<std::map<std::string,std::string>> rows, int departmentId, bool ignoreDuplicates = false);

    // Constructor to create class from database
    EmployeeData(int employeeId, Database* db_ptr);
    
    // Setter function
    void setValue (std::string fieldName, std::string fieldValue);
    
    // Getter functions
    std::string getFirstName(){return records[index].firstName;}
    std::string getLastName(){return records[index].lastName;}
    std::string getEmail(){return records[index].email;}
    std::string getPassword(){return records[index].password;}
    std::string getPasswordSalt(){return records[index].passwordSalt;}
    int getDepartmentId(){return records[index].departmentId;}
    bool isAdmin(){return records[index].admin;}

protected:
    std::size_t recordCount() const override { return records.size(); }

private:
    struct Record{
        int id = 0;
        std::string firstName;
        std::string lastName;
        std::string email;
        std::string password;
        std::string passwordSalt;
        int departmentId;
        bool admin;
    };
    std::vector<Record> records;
    Database& db;

    void validate();
    void validateRecord(Record& record);
};

} // namespace data

#endif // EMPLOYEEDATA_H
