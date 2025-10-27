#include "../include/Delete_Command.h"
#include "../include/DataType_Int.h"
#include "../include/DataType_Varchar.h"
#include "../include/DataType_Date.h"

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos) return "";
    
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, last - first + 1);
}

void Delete_Command::parseCommand(std::string command) {
    std::istringstream ss(command);
    std::string word;
    
    ss >> word;  
    if (word == "DELETE") {
        ss >> word;  
        if (word == "FROM") {
            ss >> word;  
            this->tableName = word;
            
            ss >> word;  
            if (word == "WHERE") {
                std::string whereClause;
                std::getline(ss, whereClause);
                
                size_t pos = whereClause.find('=');
                if (pos != std::string::npos) {
                    this->whereColumn = trim(whereClause.substr(0, pos));
                    this->whereValue = trim(whereClause.substr(pos + 1));
                }
            }
        }
    }
}

std::string Delete_Command::execute(Database &db)
{
    auto table = db.getTable(tableName);
    std::string pkColumn = table->getPrimaryKeyColumn();
    if (pkColumn.empty()) 
    {
        return "ERROR: Table has no PRIMARY KEY";
    }

    const Column*pkCol = table->getColumn(pkColumn);
    if(!pkCol)
    {
        return "ERROR: column not found";
    }

    std::shared_ptr<DataType_Interface> pkValue;
    if(pkCol->getType()== "INT")
    {
        pkValue=std::make_shared<DataType_Int>(std::stoi(whereValue));
    }
    else if(pkCol->getType()== "VARCHAR")
    {
        pkValue = std::make_shared<DataType_Varchar>(whereValue, pkCol->getMaxLen());
    }
    else if(pkCol->getType()== "DATE")
    {
        pkValue = std::make_shared<DataType_Date>(whereValue);
    }
    else 
    {
        return "ERROR: tip de date inexistent";
    }
    
    if (table->deleteRow(pkValue)) 
    {
        return "SUCCESS: Row deleted";
    } else 
    {
        return "ERROR: Row not found";
    }
}
