#include "../include/Insert_Command.h"
#include "../include/DataType_Int.h"
#include "../include/DataType_Varchar.h"
#include "../include/DataType_Date.h"
#include <sstream>
#include <string>
#include <algorithm>

void Insert_Command::parseCommand(std::string command)
{
    std::istringstream ss(command);
    std::string word;
    
    ss>>word;
    if(word=="INSERT")
    {
        ss>>word;
        if(word=="INTO")
        {
            ss>>word;
            this->tableName=word;
            ss>>word;
            if(word=="VALUES")
            {
                std::string rest;
                std::getline(ss, rest);
                size_t start=rest.find('(');
                size_t end = rest.find(')');

                if (start != std::string::npos && end != std::string::npos)
                {
                    std::string valuesStr = rest.substr(start + 1, end - start - 1);
                    std::istringstream valuesStream(valuesStr);
                    std::string value;

                    while (std::getline(valuesStream, value, ','))
                    {
                        value.erase(0, value.find_first_not_of(" \t"));
                        value.erase(value.find_last_not_of(" \t") + 1);

                        if (!value.empty() && value.front() == '\'' && value.back() == '\'')
                        {
                             value = value.substr(1, value.length() - 2);
                        }
                        this->fields.push_back(value);
                    }

                }
            }
        }
        
        
    }
}

std::string Insert_Command::execute(Database& db) 
{
   
    auto table = db.getTable(tableName);
    if (!table) {
        return "ERROR: Table '" + tableName + "' not found";
    }

    const auto& columns= table->getColumns();
    if (fields.size() != columns.size()) 
    {
        return "ERROR: Expected " + std::to_string(columns.size()) + " values, got " + std::to_string(fields.size());
    }

    std::map<std::string, std::shared_ptr<DataType_Interface>> rowData;

    for (size_t i = 0; i < columns.size(); i++) {
        const Column& col = columns[i];
        std::string value = fields[i];
        if (col.getType() == "INT") {
            try 
            {
                rowData[col.getName()] = std::make_shared<DataType_Int>(std::stoi(value));
            } 
            catch (...)
            {
                return "ERROR: Invalid INT value for column '" + col.getName() + "': " + value;
            }
            
        } 
        else if (col.getType() == "VARCHAR") 
        {
            if (value.length() > col.getMaxLen()) 
            {
                return "ERROR: VARCHAR too long for column '" + col.getName() + "' (max " + std::to_string(col.getMaxLen()) + ")";
            }
            rowData[col.getName()] = std::make_shared<DataType_Varchar>(value, col.getMaxLen());
            
        } 
        else if (col.getType() == "DATE") 
        {
            if (value.length() != 10 || value[4] != '-' || value[7] != '-') 
            {
                return "ERROR: Invalid DATE format for column '" + col.getName() + "' (use YYYY-MM-DD)";
            }
            rowData[col.getName()] = std::make_shared<DataType_Date>(value);
            
        } 
        else 
        {
            return "ERROR: Unsupported data type: " + col.getType();
        }
    }
    
    if (table->insertRow(rowData)) 
    {
        return "SUCCESS: Row inserted";
    } 
    else 
    {
        return "ERROR: Insert failed";
    }
}