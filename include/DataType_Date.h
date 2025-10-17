#pragma once
#include "DataType.h"
#include <stdexcept>
#include <ctime>
#include <sstream>
#include <iomanip>

class DataType_Date : public DataType
{
private:
    std::string dateString;  // Format: an:luna:zi
    
public:
    DataType_Date(const std::string& date = "1970-01-01") : dateString(date) {
        if (!isValidDate(date)) {
            throw std::runtime_error("Invalid date format. Use YYYY-MM-DD");
        }
    }
    
    //getter
    std::string getValue() const { return dateString; }
    //setter
    void setValue(const std::string& date) { 
        if (!isValidDate(date)) {
            throw std::runtime_error("Invalid date format. Use YYYY-MM-DD");
        }
        dateString = date; 
    }
    
    bool isValidDate(const std::string& date) const {
        if (date.length() != 10) return false;
        if (date[4] != '-' || date[7] != '-') return false;
        return true;
    }
    
    bool operator<(const DataType_Interface& other) const override;
    bool operator>(const DataType_Interface& other) const override;
    bool operator==(const DataType_Interface& other) const override;
    bool operator<=(const DataType_Interface& other) const override;
    bool operator>=(const DataType_Interface& other) const override;
    
    std::string toString() const override { return dateString; }
    std::string getType() const override { return "DATE"; }
    
    std::unique_ptr<DataType_Interface> clone() const override {
        return std::make_unique<DataType_Date>(dateString);
    }
};