#ifndef ARTEFACTDATA_H
#define ARTEFACTDATA_H

#include <string>
#include <vector>
#include <map>

#include "database.h"
#include "datarecord.h"

namespace data {

class ArtefactData : public DataRecord {
public:
    // Constructor to manually create class with a given artefact type ID
    ArtefactData(Database& db, int artefactTypeId, int projectId);
    
    // Constructor to create class using a vector of maps
    ArtefactData(Database& db, std::vector<std::map<std::string, std::string>> rows, int artefactTypeId, bool ignoreDuplicates = false);

    // Constructor to create class from database
    ArtefactData(int artefactId, Database* db_ptr);

    // Setter function
    void setValue(std::string fieldName, std::string fieldValue);

    // Getter functions
    std::string getName(){return records[index].title;}
    int getArtefactOwnerId(){return records[index].artefactOwnerId;}
    int getArtefactTypeId(){return records[index].artefactTypeId;}

protected:
    std::size_t recordCount() const override { return records.size(); }

private:
    struct Record{
        int id = 0;
        int artefactTypeId;
        std::string title;
        int artefactOwnerId;
    };
    std::vector<Record> records;
    Database& db;

    void validate();
    void validateRecord(Record& record);
};

} // namespace data

#endif // ARTEFACTDATA_H
