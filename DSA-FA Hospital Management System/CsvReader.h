#pragma once
#include <string>
#include <vector>
#include <stdexcept>
namespace CsvReader {
inline std::string trim(const std::string& value) {
    auto first=value.find_first_not_of(" \t\r");
    return first==std::string::npos ? std::string{} : value.substr(first,value.find_last_not_of(" \t\r")-first+1);
}
inline std::vector<std::string> fields(const std::string& line) {
    if (line.size()>4000) throw std::runtime_error("CSV line too long");
    std::vector<std::string> fields;
    size_t pos=0;
    for (;;) {
        while (pos<line.size() && line[pos]==' ') ++pos;
        std::string value;
        if (pos<line.size() && line[pos]=='"') {
            ++pos; bool closed=false;
            while (pos<line.size()) {
                char c=line[pos++];
                if (c!='"') value+=c;
                else if (pos<line.size() && line[pos]=='"') {value+='"';++pos;}
                else {closed=true;break;}
            }
            if (!closed) throw std::runtime_error("Unclosed CSV quote");
            while (pos<line.size() && (line[pos]==' ' || line[pos]=='\r')) ++pos;
            if (pos<line.size() && line[pos]!=',') throw std::runtime_error("Invalid quoted field");
        } else {
            while (pos<line.size() && line[pos]!=',') {
                if (line[pos]=='"') throw std::runtime_error("Invalid CSV quote");
                value+=line[pos++];
            }
        }
        fields.push_back(trim(value));
        if (pos==line.size()) break;
        ++pos;
    }
    return fields;
}
}
