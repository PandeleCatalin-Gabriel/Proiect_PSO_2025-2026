#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "Command_Interface.h"

class Select_Command : public Command_Interface
{
private:
    std::string tableName;
    bool selectAll;
    std::vector<std::string> selectedColumns;
    bool hasWhere;
    std::string whereColumn;
    std::string whereValue;
    
public:
    Select_Command() : selectAll(false), hasWhere(false) {}
    
    void parseCommand(std::string command);
    std::string execute(Database& db) override;
    
    std::string getTableName() const { return tableName; }
    bool getSelectAll() const { return selectAll; }
    const std::vector<std::string>& getSelectedColumns() const { return selectedColumns; }
    bool getHasWhere() const { return hasWhere; }
    std::string getWhereColumn() const { return whereColumn; }
    std::string getWhereValue() const { return whereValue; }
};