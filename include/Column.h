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
    Column(const std::string& n ="", const std::string & type= "",
    int maxLen=0, bool isPK= false, bool nullable= true)
    : name(n),type(type),maxLength(maxLen),isNull(nullable),
    isPrimaryKey(isPK){}
    
    std::string getName()const;
    std::string getType()const;
    int getMaxLen()const;
    bool getIsPrimaryKey()const;
    bool getIsNullable()const;
    bool getHasIndex()const;
};