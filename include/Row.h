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
    
};