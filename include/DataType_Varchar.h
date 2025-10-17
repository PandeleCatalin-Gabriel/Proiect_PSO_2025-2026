#pragma once
#include"DataType.h"
#include<string.h>
#include<iostream>

class DataType_Varchar : public DataType
{
private:
    std::string varcharString;
    size_t maxLen;
public:
    DataType_Varchar(const std::string & varcharString="", size_t maxLen= 255)
    :varcharString(varcharString),maxLen(maxLen) {}

    //getter
    std::string getValue()const;
    size_t getMaxLen()const;
    //setter
    void setValue(const std::string & varcharString);

    //cmp pt b-trees
    bool operator<(const DataType_Interface& other) const override;
    bool operator>(const DataType_Interface& other) const override;
    bool operator==(const DataType_Interface& other) const override;
    bool operator<=(const DataType_Interface& other) const override;
    bool operator>=(const DataType_Interface& other) const override;

    std::string toString()const override;
    std::string getType()const override;

    std::unique_ptr<DataType_Interface> clone()const override;

};