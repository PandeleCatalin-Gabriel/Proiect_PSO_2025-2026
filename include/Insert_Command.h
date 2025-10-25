#pragma once
#include <string>
#include <map>
#include <memory>
#include "Command_Interface.h"
#include "DataType_Interface.h"
#include<vector>


class Insert_Command : public Command_Interface
{
private:
    std::string tableName;
    std::vector<std::string> fields;
    
public:
    Insert_Command(){};

    std::string getTableName() const { return tableName; }
    const std::vector<std::string>& getFields() const { return fields; }

    void parseCommand(std::string command);
    std::string execute(Database& db)override;
};