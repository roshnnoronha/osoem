#ifndef ARTEFACTTOACTIVITYLINKDATA_H
#define ARTEFACTTOACTIVITYLINKDATA_H

#include <string>
#include <vector>
#include <map>

#include "database.h"
#include "datarecord.h"

namespace data {

class ArtefactToActivityLinkData : public DataRecord {
public:
    // Constructor to manually create class
    ArtefactToActivityLinkData(Database& db, int artefactId, int projectId);

    // Constructor to create class using a vector of maps
    ArtefactToActivityLinkData(Database& db, std::vector<std::map<std::string,std::string>> rows, int artefactId, int projectId, bool ignoreDuplicates = false);

    // Constructor to create class from database
    ArtefactToActivityLinkData(int artefactLinkId, Database* db_ptr);

    // Setter function
    void setValue(std::string fieldName, std::string fieldValue);

    // Getter functions
    int getActivityId() { return records[index].activityId; }
    int getArtefactId() { return records[index].artefactId; }
    double getRatio() { return records[index].ratio; }

protected:
    std::size_t recordCount() const override { return records.size(); }

private:
    struct Record {
        int id = 0;
        int activityId;
        int artefactId;
        double ratio;
    };
    std::vector<Record> records;
    Database& db;

    void validate();
    void validateRecord(Record& record);
};

} // namespace data

#endif // ARTEFACTTOACTIVITYLINKDATA_H
