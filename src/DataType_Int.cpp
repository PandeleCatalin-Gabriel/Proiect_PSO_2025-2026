#include "DataType_Int.h"
#include<stdexcept>

int DataType_Int::getValue() const
{
    return this->value;
}

void DataType_Int::setValue(int val)
{
    this->value=val;
}

bool DataType_Int::operator<(const DataType_Interface &other) const
{
    //conversie la tipul de date
    auto otherInt=dynamic_cast<const DataType_Int*>(&other);
    if(!otherInt)
    {
        throw std::runtime_error("Tip de date eronat");
    }
    if(this->value < otherInt->value)
    {
        return true;
    }
    return false;
}

bool DataType_Int::operator>(const DataType_Interface &other) const
{
    auto otherInt=dynamic_cast<const DataType_Int*>(&other);
    if(!otherInt)
    {
        throw std::runtime_error("Tip de date eronat");
    }
    if(this->value > otherInt->value)
    {
        return true;
    }
    return false;
}

bool DataType_Int::operator==(const DataType_Interface &other) const
{
    auto otherInt=dynamic_cast<const DataType_Int*>(&other);
    if(!otherInt)
    {
        throw std::runtime_error("Tip de date eronat");
    }
    if(this->value == otherInt->value)
    {
        return true;
    }
    return false;
}

bool DataType_Int::operator<=(const DataType_Interface &other) const
{
    auto otherInt=dynamic_cast<const DataType_Int*>(&other);
    if(!otherInt)
    {
        throw std::runtime_error("Tip de date eronat");
    }
    if(this->value <= otherInt->value)
    {
        return true;
    }
    return false;
}

bool DataType_Int::operator>=(const DataType_Interface &other) const
{
    auto otherInt=dynamic_cast<const DataType_Int*>(&other);
    if(!otherInt)
    {
        throw std::runtime_error("Tip de date eronat");
    }
    if(this->value >= otherInt->value)
    {
        return true;
    }
    return false;
}

std::string DataType_Int::toString() const
{
    return std::to_string(value);
}

std::string DataType_Int::getType() const
{
    return "INT";
}

std::unique_ptr<DataType_Interface> DataType_Int::clone() const
{
    return std::make_unique<DataType_Int>(value);
}
