#ifndef FIELDDATA_H
#define FIELDDATA_H

#include <string>
#include <vector>
#include <map>
#include "database.h"
#include "datarecord.h"

namespace data {

class FieldData : public DataRecord {
public:
    // Constructor to manually create class
    FieldData(Database& db, int artefactTypeId);
    
    // Constructor to create class using a vector of maps
    FieldData(Database& db, std::vector<std::map<std::string,std::string>> rows, int artefactTypeId, bool ignoreDuplicates = false);

    // Constructor to create class from database
    FieldData( int fieldId,Database* db_ptr);

    // Setter function
    void setValue(std::string fieldName, std::string fieldValue);

    // Getter functions
    std::string getTitle(){return records[index].title;}
    int getValueType(){return records[index].valueType;}
    int getMaimumLength(){return records[index].maximumLength;}
    int getMaximumValue(){return records[index].maximumValue;}
    int getMinimumValue(){return records[index].minimumValue;}
    int getArtefactTypeId(){return records[index].artefactTypeId;}

protected:
    std::size_t recordCount() const override { return records.size(); }

private:
    struct Record{
        int id = 0;
        int artefactTypeId;
        std::string title;
        int valueType;
        int maximumLength;
        int maximumValue;
        int minimumValue;
    };
    std::vector<Record> records;
    Database& db;

    void validate();
    void validateRecord(Record& record);
};

} // namespace data

#endif // FIELDDATA_H
