#pragma once
#include"DataType.h"
#include<string>
#include<memory>
#include<iostream>

class DataType_Int : public DataType
{
private:
    int value;
public:
    DataType_Int(int val =0):value(val){}

    //getter
    int getValue()const;
    //setter
    void setValue(int val);

    //operatori b-trees
    bool operator<(const DataType_Interface & other)const override;
    bool operator>(const DataType_Interface & other)const override;
    bool operator==(const DataType_Interface & other)const override;
    bool operator<=(const DataType_Interface & other)const override;
    bool operator>=(const DataType_Interface & other)const override;

    std::string toString()const override;
    std::string getType()const override;

    std::unique_ptr<DataType_Interface> clone()const override;
};