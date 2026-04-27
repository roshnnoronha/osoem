#ifndef ACTIVITYDATA_H
#define ACTIVITYDATA_H

#include <string>
#include <vector>
#include <map>

#include "database.h"
#include "datarecord.h"

namespace data {

class ActivityData : public DataRecord {
public:
    // Constructor to manually create class with given subcategory Id
    ActivityData(Database& db, int subCategoryId, int projectId);

    // Constructor to create class using a vector of maps
    ActivityData(Database& db, std::vector<std::map<std::string,std::string>> rows, int subCategoryId, bool ignoreDuplicates = false);

    // Constructor to create class from database
    ActivityData(int activityId, Database* db_ptr);

    // Setter function
    void setValue(std::string fieldName, std::string fieldValue);

    // Getter functions
    std::string getName(){return records[index].name;}
    std::string getDescription(){return records[index].description;}
    int getSubCategoryId(){return records[index].subCategoryId;}
    int getManagerId(){return records[index].managerId;}
    std::string getPlannedStart(){return records[index].plannedStart;}
    std::string getPlannedEnd(){return records[index].plannedEnd;}
    std::string getForecastStart(){return records[index].forecastStart;}
    std::string getForecastEnd(){return records[index].forecastEnd;}
    std::string getActualEnd(){return records[index].actualEnd;}
    double getPlannedHours(){return records[index].plannedHours;}
    double getForecastHours(){return records[index].forecastHours;}

protected:
    std::size_t recordCount() const override { return records.size(); }

private:
    struct Record{
        int id = 0;
        std::string name;
        std::string description;
        int subCategoryId;
        int managerId;
        std::string plannedStart;
        std::string plannedEnd;
        std::string forecastStart;
        std::string forecastEnd;
        std::string actualEnd;
        double plannedHours;
        double forecastHours;
    };
    std::vector<Record> records;
    Database& db;

    void validate();
    void validateRecord(Record& record);
};

} // namespace data

#endif // ACTIVITYDATA_H
