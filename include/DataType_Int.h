#pragma once
#include"DataType.h"

class DataType_Int : public DataType
{
private:
    int value;
public:
    DataType_Int(int val =0):value(val){}

    //getters
    int getValue()const;
    //setters
    void setValue(int val);

    //operatori b-trees
    bool operator<(const DataType_Interface & other)const override;
    bool operator>(const DataType_Interface & other)const override;
    bool operator==(const DataType_Interface & other)const override;
    bool operator<=(const DataType_Interface & other)const override;
    bool operator>=(const DataType_Interface & other)const override;
};