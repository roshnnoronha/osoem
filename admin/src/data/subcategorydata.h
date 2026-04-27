#ifndef SUBCATEGORYDATA_H
#define SUBCATEGORYDATA_H

#include <string>
#include <vector>
#include <map>

#include "database.h"
#include "datarecord.h"

namespace data {

class SubCategoryData : public DataRecord {
public:
    // Constructor to manually create class
    SubCategoryData(Database& db, int categoryId);

    // Constructor to create class using a vector of maps
    SubCategoryData(Database& db, std::vector<std::map<std::string,std::string>> rows, int categoryId, bool ignoreDuplicates = false);

    // Constructor to create class from database
    SubCategoryData( int subcategoryId , Database* db_ptr);

    // Setter function
    void setValue(std::string fieldName, std::string fieldValue);

    // Getter functions
    std::string getName(){return records[index].name;}
    int getCategoryId(){return records[index].categoryId;}

protected:
    std::size_t recordCount() const override { return records.size(); }

private:
    struct Record{
        int id = 0;
        std::string name;
        int categoryId;
    };
    std::vector<Record> records;
    Database& db;

    void validate();
    void validateRecord(Record& record);
};

} // namespace data

#endif // SUBCATEGORYDATA_H
