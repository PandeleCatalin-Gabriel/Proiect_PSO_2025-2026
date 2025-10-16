#pragma once
#include<string>
#include<vector>
#include<stdbool.h>

class Column
{
private:
    std::string name;
    std::string type;
    int maxLength;
    bool isPrimaryKey;
    bool isNull;
    bool hasIndex;
public:
    
};