#pragma once
#include<iostream>
#include<string>
#include<vector>
#include"DataType_Interface.h"
#include<map>
#include <memory>

class Row
{
private:
    int rowID;
    std::map<std::string,std::shared_ptr<DataType_Interface>> data;
public:
    Row(int ID=0): rowID(ID){}

    //getters
    int getRowID()const;
    std::map<std::string,std::shared_ptr<DataType_Interface>> getData()const;

    //setters
    void setRowID(int ID);
    std::shared_ptr<DataType_Interface> getValue(const std::string& columnName);
    void setValue(const std::string& columnName, std::shared_ptr<DataType_Interface> value);
    bool hasColumn(const std::string & columnName)const;
};