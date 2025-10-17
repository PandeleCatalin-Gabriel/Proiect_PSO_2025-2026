#include "../include/DataType_Varchar.h"
#include<string>
#include<iostream>
#include<stdexcept>

std::string DataType_Varchar::getValue() const
{
    return this->varcharString;
}
size_t DataType_Varchar::getMaxLen() const
{
    return this->maxLen;
}

void DataType_Varchar::setValue(const std::string &varcharString)
{
    this->varcharString=varcharString;
}

bool DataType_Varchar::operator<(const DataType_Interface &other) const
{
    auto otherVarchar=dynamic_cast<const DataType_Varchar*>(&other);
    if(!otherVarchar)
    {
        throw std::runtime_error("Tip de date eronat");
    }
    if(this->varcharString< otherVarchar->varcharString)
    {
        return true;
    }
    return false;
}

bool DataType_Varchar::operator>(const DataType_Interface &other) const
{
    auto otherVarchar=dynamic_cast<const DataType_Varchar*>(&other);
    if(!otherVarchar)
    {
        throw std::runtime_error("Tip de date eronat");
    }
    if(this->varcharString> otherVarchar->varcharString)
    {
        return true;
    }
    return false;
}

bool DataType_Varchar::operator==(const DataType_Interface &other) const
{
    auto otherVarchar=dynamic_cast<const DataType_Varchar*>(&other);
    if(!otherVarchar)
    {
        throw std::runtime_error("Tip de date eronat");
    }
    if(this->varcharString == otherVarchar->varcharString)
    {
        return true;
    }
    return false;
}

bool DataType_Varchar::operator<=(const DataType_Interface &other) const
{
    auto otherVarchar=dynamic_cast<const DataType_Varchar*>(&other);
    if(!otherVarchar)
    {
        throw std::runtime_error("Tip de date eronat");
    }
    if(this->varcharString <= otherVarchar->varcharString)
    {
        return true;
    }
    return false;
}

bool DataType_Varchar::operator>=(const DataType_Interface &other) const
{
    auto otherVarchar=dynamic_cast<const DataType_Varchar*>(&other);
    if(!otherVarchar)
    {
        throw std::runtime_error("Tip de date eronat");
    }
    if(this->varcharString >= otherVarchar->varcharString)
    {
        return true;
    }
    return false;
}

std::string DataType_Varchar::toString() const
{
    return varcharString;
}

std::string DataType_Varchar::getType() const
{
    return "VARCHAR";
}

std::unique_ptr<DataType_Interface> DataType_Varchar::clone() const
{
    return std::make_unique<DataType_Varchar>(varcharString,maxLen);
}
