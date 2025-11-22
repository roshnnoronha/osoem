#include "csv_parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

std::vector<std::map<std::string, std::string>> CSVParser::parse(const std::string& filename) {
    std::vector<std::map<std::string, std::string>> rows;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return rows;
    }

    std::string line;
    std::vector<std::string> headers;

    // Read header line
    if (std::getline(file, line)) {
        headers = parseLine(line);
        // Trim headers
        for (auto& header : headers) {
            header = trim(header);
        }
    }

    // Read data lines
    while (std::getline(file, line)) {
        // Skip empty lines
        if (trim(line).empty()) {
            continue;
        }

        std::vector<std::string> values = parseLine(line);

        if (values.size() != headers.size()) {
            std::cerr << "Warning: Line has " << values.size()
                      << " fields but expected " << headers.size()
                      << ". Skipping line." << std::endl;
            continue;
        }

        std::map<std::string, std::string> row;
        for (size_t i = 0; i < headers.size(); i++) {
            row[headers[i]] = trim(values[i]);
        }
        rows.push_back(row);
    }

    file.close();
    return rows;
}

std::vector<std::string> CSVParser::parseLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;

    for (size_t i = 0; i < line.length(); i++) {
        char c = line[i];

        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            fields.push_back(field);
            field.clear();
        } else {
            field += c;
        }
    }

    // Add the last field
    fields.push_back(field);

    return fields;
}

std::string CSVParser::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}
