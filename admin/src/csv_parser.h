#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <string>
#include <vector>
#include <map>

class CSVParser {
public:
    // Parse a CSV file and return rows as vector of maps (column_name -> value)
    static std::vector<std::map<std::string, std::string>> parse(const std::string& filename);

private:
    // Parse a single CSV line handling quoted fields
    static std::vector<std::string> parseLine(const std::string& line);

    // Trim whitespace from string
    static std::string trim(const std::string& str);
};

#endif // CSV_PARSER_H
