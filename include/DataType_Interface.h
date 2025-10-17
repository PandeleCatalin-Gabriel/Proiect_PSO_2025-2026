#pragma once
#include<memory>

class DataType_Interface
{
public:
    //destructor virtual
    virtual ~DataType_Interface()=default;

    //comparari pt b-trees
    virtual bool operator<(const DataType_Interface& other) const = 0;
    virtual bool operator>(const DataType_Interface& other) const = 0;
    virtual bool operator==(const DataType_Interface& other) const = 0;
    virtual bool operator<=(const DataType_Interface& other) const = 0;
    virtual bool operator>=(const DataType_Interface& other) const = 0;

    virtual std::string toString() const = 0;
    virtual std::string getType() const = 0;

    virtual std::unique_ptr<DataType_Interface> clone() const = 0;
};