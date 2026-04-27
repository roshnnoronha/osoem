#ifndef ARTEFACTDATAITEMDATA_H
#define ARTEFACTDATAITEMDATA_H

#include <string>
#include <vector>
#include <map>

#include "database.h"
#include "datarecord.h"

namespace data {

class ArtefactDataItemData : public DataRecord {
public:
    // Constructor to manually create class with a given deparment ID
    ArtefactDataItemData(Database& db, int artefactId, int artefactTypeId );

    // Constructor to create class using a vector of maps
    ArtefactDataItemData(Database& db, std::vector<std::map<std::string,std::string>> rows, int artefactId, int artefactTypeId, bool ignoreDuplicates = false);

    // Constructor to create class from database
    ArtefactDataItemData( int artefactDataId, Database* db_ptr);

    // Setter function
    void setValue(std::string fieldName, std::string fieldValue);

    // Getter functions
    int getArtefactId() { return records[index].artefactId; }
    int getArtefactDataFieldId() { return records[index].artefactDataFieldId; }
    std::string getValue() { return records[index].value; }

protected:
    std::size_t recordCount() const override { return records.size(); }

private:
    struct Record {
        int id = 0;
        int artefactId;
        int artefactDataFieldId;
        std::string value;
    };
    std::vector<Record> records;
    Database& db;

    void validate();
    void validateRecord(Record& record);
};

} // namespace data

#endif // ARTEFACTDATAITEMDATA_H
