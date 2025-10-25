#pragma once
#include <string>
#include <memory>
#include "Command_Interface.h"
#include "DataType_Interface.h"

class Select_Command : public Command_Interface
{
private:
    std::string tableName;
    std::string whereColumn;
    std::shared_ptr<DataType_Interface> whereValue;
    bool selectAll;
    
public:
    Select_Command(const std::string& table)
    : tableName(table), selectAll(true) {}
    
    Select_Command(const std::string& table, const std::string& col, 
    const std::shared_ptr<DataType_Interface>& val)
    : tableName(table), whereColumn(col), whereValue(val), selectAll(false) {}
    
    std::string execute(Database& db) override;
};