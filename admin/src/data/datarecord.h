#ifndef DATARECORD_H
#define DATARECORD_H

#include <cstddef>

namespace data {

/**
 * Abstract base class for all data record classes.
 * Provides a common interface and implementation for iterating through records.
 */
class DataRecord {
public:
    virtual ~DataRecord() = default;

    /**
     * Advances to the next record.
     * @return true if there is a next record, false if at end
     */
    bool next() {
        if (index < recordCount() - 1) {
            index++;
            return true;
        }
        return false;
    }

    /**
     * Resets the iterator to the first record.
     */
    void reset() {
        index = 0;
    }

protected:
    std::size_t index = 0;

    DataRecord() = default;
    DataRecord(const DataRecord&) = default;
    DataRecord& operator=(const DataRecord&) = default;

    /**
     * Returns the number of records. Must be implemented by derived classes.
     */
    virtual std::size_t recordCount() const = 0;
};

} // namespace data

#endif // DATARECORD_H
