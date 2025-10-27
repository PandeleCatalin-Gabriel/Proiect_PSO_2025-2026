#include "../include/Select_Command.h"
#include "../include/DataType_Int.h"
#include "../include/DataType_Varchar.h"
#include "../include/DataType_Date.h"
#include <sstream>
#include <algorithm>

static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, last - first + 1);
}

void Select_Command::parseCommand(std::string command) {
    std::istringstream ss(command);
    std::string word;
    
    ss >> word;
    if (word != "SELECT") return;
    
    std::string columnsStr;
    std::string nextWord;
    
    while (ss >> nextWord && nextWord != "FROM") {
        columnsStr += nextWord + " ";
    }
    
    columnsStr = trim(columnsStr);
    
    if (columnsStr == "*") {
        this->selectAll = true;
    } else {
        this->selectAll = false;
        
        std::istringstream colStream(columnsStr);
        std::string col;
        while (std::getline(colStream, col, ',')) {
            this->selectedColumns.push_back(trim(col));
        }
    }
    
    if (nextWord == "FROM") {
        ss >> this->tableName;
        
        if (ss >> word && word == "WHERE") {
            this->hasWhere = true;
            
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

std::string Select_Command::execute(Database& db) {
    auto table = db.getTable(tableName);
    if (!table) {
        return "ERROR: Table not found";
    }
    
    std::vector<std::shared_ptr<Row>> rows;
    
    if (!hasWhere) {
        rows = table->selectAll();
    } else {
        std::string pkColumn = table->getPrimaryKeyColumn();
        if (pkColumn.empty()) {
            return "ERROR: Table has no PRIMARY KEY";
        }
        
        if (whereColumn != pkColumn) {
            return "ERROR: SELECT WHERE only supports PRIMARY KEY";
        }
        
        const Column* pkCol = table->getColumn(pkColumn);
        if (!pkCol) {
            return "ERROR: Column not found";
        }
        
        std::shared_ptr<DataType_Interface> pkValue;
        
        if (pkCol->getType() == "INT") {
            pkValue = std::make_shared<DataType_Int>(std::stoi(whereValue));
        } else if (pkCol->getType() == "VARCHAR") {
            pkValue = std::make_shared<DataType_Varchar>(whereValue, pkCol->getMaxLen());
        } else if (pkCol->getType() == "DATE") {
            pkValue = std::make_shared<DataType_Date>(whereValue);
        } else {
            return "ERROR: Unsupported data type";
        }
        
        auto row = table->selectByPrimaryKey(pkValue);
        if (row) {
            rows.push_back(row);
        }
    }
    
    if (rows.empty()) {
        return "0 rows";
    }
    
    std::string result = std::to_string(rows.size()) + " row(s):\n";
    
    for (const auto& row : rows) {
        result += "Row " + std::to_string(row->getRowID()) + ": ";
        
        if (selectAll) {
            for (const auto& [colName, value] : row->getData()) {
                result += colName + "=" + value->toString() + " ";
            }
        } else {
            for (const auto& colName : selectedColumns) {
                auto value = row->getValue(colName);
                if (value) {
                    result += colName + "=" + value->toString() + " ";
                } else {
                    result += colName + "=NULL ";
                }
            }
        }
        
        result += "\n";
    }
    
    return result;
}