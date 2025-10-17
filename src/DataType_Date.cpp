#include "../include/DataType_Date.h"
#include <stdexcept>

bool DataType_Date::operator<(const DataType_Interface& other) const {
    auto otherDate = dynamic_cast<const DataType_Date*>(&other);
    if (!otherDate) {
        throw std::runtime_error("Tip de date eronat");
    }
    if(this->dateString < otherDate->dateString)
    {
        return true;
    }
    return false;
}

bool DataType_Date::operator>(const DataType_Interface& other) const {
    auto otherDate = dynamic_cast<const DataType_Date*>(&other);
    if (!otherDate) {
        throw std::runtime_error("Tip de date eronat");
    }
    if (this->dateString > otherDate->dateString)
    {
        return true;
    }
    return false;
}

bool DataType_Date::operator==(const DataType_Interface& other) const {
    auto otherDate = dynamic_cast<const DataType_Date*>(&other);
    if (!otherDate) {
        throw std::runtime_error("Tip de date eronat");
    }
    if(this->dateString == otherDate->dateString)
    {
        return true;
    }
    return false;
}

bool DataType_Date::operator<=(const DataType_Interface& other) const {
    auto otherDate = dynamic_cast<const DataType_Date*>(&other);
    if (!otherDate) {
        throw std::runtime_error("Tip de date eronat");
    }
    if(this->dateString <= otherDate->dateString)
    {
        return true;
    }
    return false;
}

bool DataType_Date::operator>=(const DataType_Interface& other) const {
    auto otherDate = dynamic_cast<const DataType_Date*>(&other);
    if (!otherDate) {
        throw std::runtime_error("Tip de date eronat");
    }
    if(this->dateString >= otherDate->dateString)
    {
        return true;
    }
    return false;
}