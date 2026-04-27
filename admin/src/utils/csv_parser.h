#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <string>
#include <vector>
#include <map>
namespace utils {

// Parse a comma separated line of text and return a row of maps
std::vector<std::map<std::string, std::string>> parseCSVString(const std::string& CSVString) ;

// Parse a CSV file and return rows as vector of maps (column_name -> value)
std::vector<std::map<std::string, std::string>> parseCSVFile(const std::string& filename);

// Parse a single CSV line handling quoted fields
std::vector<std::string> parseLine(const std::string& line);

// Trim whitespace from string
std::string trim(const std::string& str);

} // namespace utils
#endif // CSV_PARSER_H
