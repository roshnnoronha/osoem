#ifndef MILESTONESTEPDATA_H
#define MILESTONESTEPDATA_H

#include <string>
#include <vector>
#include <map>

#include "database.h"
#include "datarecord.h"

namespace data {

class MilestoneStepData : public DataRecord {
public:
    // Constructor to manually create class
    MilestoneStepData(Database& db, int milestoneId);

    // Constructor to create class using a vector of maps
    MilestoneStepData(Database& db, std::vector<std::map<std::string,std::string>> rows, int milestoneId, bool ignoreDuplicates = false);

    // Constructor to create class from database
    MilestoneStepData( int milestoneStepId , Database* db);

    // Setter function
    void setValue(std::string fieldName, std::string fieldValue);

    // Getter functions
    std::string getName(){return records[index].name;}
    double getProgressRatio(){return records[index].progressRatio;}
    int getMilestoneId(){return records[index].milestoneId;}

protected:
    std::size_t recordCount() const override { return records.size(); }

private:
    struct Record{
        int id = 0;
        int milestoneId;
        std::string name;
        double progressRatio;
    };
    std::vector<Record> records;
    Database& db;

    void validate();
    void validateRecord(Record& record);
};

} // namespace data

#endif // MILESTONESTEPDATA_H
