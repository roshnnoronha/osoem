#ifndef HOURSDATA_H
#define HOURSDATA_H

#include <string>
#include <vector>
#include <map>

#include "database.h"
#include "datarecord.h"

namespace data {

class HoursData : public DataRecord {
public:
    // Constructor to manually create class
    HoursData(Database& db, int assignmentId);

    // Constructor to create class using a vector of maps
    HoursData(Database& db, std::vector<std::map<std::string,std::string>> rows, int assignmentId, bool ignoreDuplicates = false);

    // Constructor to create class from database
    HoursData( int bookingId, Database* db_ptr);

    // Setter function
    void setValue(std::string fieldName, std::string fieldValue);

    // Getter functions
    int getAssignmentId() { return records[index].assignmentId; }
    std::string getBookedDate() { return records[index].bookedDate; }
    int getHours() { return records[index].hours; }

protected:
    std::size_t recordCount() const override { return records.size(); }

private:
    struct Record {
        int id = 0;
        int assignmentId;
        std::string bookedDate;
        int hours;
    };
    std::vector<Record> records;
    Database& db;

    void validate();
    void validateRecord(Record& record);
};

} // namespace data

#endif // HOURSDATA_H
