#ifndef NOTEDATA_H
#define NOTEDATA_H

#include <string>
#include <vector>
#include <map>

#include "database.h"
#include "datarecord.h"

namespace data {

class NoteData : public DataRecord {
public:
    // Constructor to manually create class
    NoteData(Database& db, int activityId, int projectId);

    // Constructor to create class using a vector of maps
    NoteData(Database& db, std::vector<std::map<std::string,std::string>> rows, int activityId, bool ignoreDuplicates = false);

    // Constructor to create class from database
    NoteData( int noteId , Database* db_ptr);

    // Setter function
    void setValue(std::string fieldName, std::string fieldValue);

    // Getter functions
    std::string getNote(){return records[index].note;}
    std::string getNoteDate(){return records[index].noteDate;}
    int getActivityId(){return records[index].activityId;}
    int getUserId(){return records[index].userId;}

protected:
    std::size_t recordCount() const override { return records.size(); }

private:
    struct Record{
        int id = 0;
        int activityId;
        int userId;
        std::string note;
        std::string noteDate; //TODO: Handle dates
    };
    std::vector<Record> records;
    Database& db;

    void validate();
    void validateRecord(Record& record);
};

} // namespace data

#endif // NOTEDATA_H
