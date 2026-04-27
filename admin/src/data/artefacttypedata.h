#ifndef ARTEFACTTYPEDATA_H
#define ARTEFACTTYPEDATA_H

#include <string>
#include <vector>
#include <map>
#include "database.h"
#include "datarecord.h"

namespace data {

class ArtefactTypeData : public DataRecord {
public:
    // Constructor to manually create class
    ArtefactTypeData(Database& db, int projectId);

    // Constructor to create class using a vector of maps
    ArtefactTypeData(Database& db, std::vector<std::map<std::string,std::string>> rows, int projectId, bool ignoreDuplicates = false);

    // Constructor to create class from database
    ArtefactTypeData( int artefactTypeId, Database* db_ptr);

    // Setter function
    void setValue(std::string fieldName, std::string fieldValue);

    // Getter functions
    std::string getName(){return records[index].name;}
    std::string getDescription(){return records[index].description;}
    int getProjectId(){return records[index].projectId;}

protected:
    std::size_t recordCount() const override { return records.size(); }

private:
    struct Record{
        int id = 0;
        int projectId;
        std::string name;
        std::string description;
    };
    std::vector<Record> records;
    Database& db;

    void validate();
    void validateRecord(Record& record);
};

} // namespace data

#endif // ARTEFACTTYPEDATA_H
