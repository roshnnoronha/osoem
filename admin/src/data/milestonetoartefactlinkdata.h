#ifndef MILESTONETOARTEFACTLINKDATA_H
#define MILESTONETOARTEFACTLINKDATA_H

#include <string>
#include <vector>
#include <map>

#include "database.h"
#include "datarecord.h"

namespace data {

class MilestoneToArtefactLinkData : public DataRecord {
public:
    // Constructor to manually create class
    MilestoneToArtefactLinkData(Database& db , int artefactLinkId, int projectId = 0);

    // Constructor to create class using a vector of maps
    MilestoneToArtefactLinkData(Database& db, std::vector<std::map<std::string,std::string>> rows, int artefactLinkId, int projectId = 0, bool ignoreDuplicates = false);

    // Constructor to create class from database
    MilestoneToArtefactLinkData( int milestoneLinkId, Database* db_ptr);

    // Setter function
    void setValue(std::string fieldName, std::string fieldValue);

    // Getter functions
    int getMilestoneStepId() { return records[index].milestoneStepId; }
    int getArtefactLinkId() { return records[index].artefactLinkId; }
    int getCompletedById() { return records[index].completedById; }
    std::string getCompletionDate() { return records[index].completionDate; }

protected:
    std::size_t recordCount() const override { return records.size(); }

private:
    struct Record {
        int id = 0;
        int milestoneStepId;
        int artefactLinkId;
        int completedById;
        std::string completionDate;
    };
    std::vector<Record> records;
    Database& db;
    int projectId = 0;

    void validate();
    void validateRecord(Record& record);
};

} // namespace data

#endif // MILESTONETOARTEFACTLINKDATA_H
