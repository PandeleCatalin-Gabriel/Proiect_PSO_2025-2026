#pragma once
#include<iostream>
#include<string>
#include "Command_Interface.h"

class Delete_Command: public Command_Interface
{
private:
    std::string tableName;
    std::string whereColumn;
    std::string whereValue;

public:
    Delete_Command(){}

    void parseCommand(std::string command);
    std::string execute(Database& db)override;

    std::string getTableName() const { return tableName; }
    std::string getWhereColumn() const { return whereColumn; }
    std::string getWhereValue() const { return whereValue; }
};