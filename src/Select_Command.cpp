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

static std::string toUpper(const std::string& str) {
    std::string upper = str;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    return upper;
}

void Select_Command::parseCommand(std::string command) {
    // Trim și uppercase pentru parsing
    command = trim(command);
    std::string cmdUpper = toUpper(command);
    
    std::cout << "Parsing SELECT command: [" << command << "]\n";
    
    // Verifică că începe cu SELECT
    if (cmdUpper.substr(0, 6) != "SELECT") {
        std::cerr << "ERROR: Not a SELECT command\n";
        return;
    }
    
    // Găsește poziția lui FROM (case insensitive)
    size_t fromPos = cmdUpper.find("FROM");
    if (fromPos == std::string::npos) {
        std::cerr << "ERROR: No FROM clause found\n";
        return;
    }
    
    std::cout << "Found FROM at position " << fromPos << "\n";
    
    // Extrage partea dintre SELECT și FROM
    std::string columnsStr = command.substr(6, fromPos - 6);
    columnsStr = trim(columnsStr);
    
    std::cout << "Columns part: [" << columnsStr << "]\n";
    
    if (columnsStr.empty()) {
        std::cerr << "ERROR: No columns specified\n";
        return;
    }
    
    // Verifică dacă e SELECT *
    if (columnsStr == "*") {
        this->selectAll = true;
        std::cout << "SELECT ALL detected\n";
    } else {
        this->selectAll = false;
        // Split by comma
        std::istringstream colStream(columnsStr);
        std::string col;
        while (std::getline(colStream, col, ',')) {
            col = trim(col);
            if (!col.empty()) {
                this->selectedColumns.push_back(col);
            }
        }
        std::cout << "Selected " << selectedColumns.size() << " columns\n";
    }
    
    // Extrage table name (după FROM, până la WHERE sau sfârșit)
    size_t tableStart = fromPos + 4; // după "FROM"
    size_t wherePos = cmdUpper.find("WHERE", tableStart);
    
    std::string tableStr;
    if (wherePos != std::string::npos) {
        tableStr = command.substr(tableStart, wherePos - tableStart);
    } else {
        tableStr = command.substr(tableStart);
    }
    
    this->tableName = trim(tableStr);
    std::cout << "Table name: [" << tableName << "]\n";
    
    // Verifică WHERE clause
    if (wherePos != std::string::npos) {
        this->hasWhere = true;
        
        std::string whereClause = command.substr(wherePos + 5); // după "WHERE"
        whereClause = trim(whereClause);
        
        std::cout << "WHERE clause: [" << whereClause << "]\n";
        
        size_t eqPos = whereClause.find('=');
        if (eqPos != std::string::npos) {
            this->whereColumn = trim(whereClause.substr(0, eqPos));
            this->whereValue = trim(whereClause.substr(eqPos + 1));
            
            // Remove quotes from value if present
            if (!this->whereValue.empty() && 
                this->whereValue.front() == '\'' && 
                this->whereValue.back() == '\'') {
                this->whereValue = this->whereValue.substr(1, this->whereValue.length() - 2);
            }
            
            std::cout << "WHERE: " << whereColumn << " = " << whereValue << "\n";
        } else {
            std::cerr << "ERROR: Invalid WHERE clause (no = found)\n";
            this->hasWhere = false;
        }
    } else {
        this->hasWhere = false;
        std::cout << "No WHERE clause\n";
    }
}

std::string Select_Command::execute(Database& db) {
    std::cout << "Executing SELECT on table: " << tableName << "\n";
    
    try {
        auto table = db.getTable(tableName);
        if (!table) {
            std::cerr << "ERROR: Table '" << tableName << "' not found\n";
            return "ERROR: Table not found";
        }
        
        std::vector<std::shared_ptr<Row>> rows;
        
        if (!hasWhere) {
            rows = table->selectAll();
            std::cout << "SELECT ALL - found " << rows.size() << " rows\n";
        } else {
            std::string pkColumn = table->getPrimaryKeyColumn();
            if (pkColumn.empty()) {
                std::cerr << "ERROR: No PRIMARY KEY\n";
                return "ERROR: Table has no PRIMARY KEY";
            }
            
            if (whereColumn != pkColumn) {
                std::cerr << "ERROR: WHERE clause must use PRIMARY KEY (" 
                         << pkColumn << "), got: " << whereColumn << "\n";
                return "ERROR: SELECT WHERE only supports PRIMARY KEY column (" + pkColumn + ")";
            }
            
            const Column* pkCol = table->getColumn(pkColumn);
            if (!pkCol) {
                std::cerr << "ERROR: Column not found\n";
                return "ERROR: Column not found";
            }
            
            std::shared_ptr<DataType_Interface> pkValue;
            
            try {
                if (pkCol->getType() == "INT") {
                    pkValue = std::make_shared<DataType_Int>(std::stoi(whereValue));
                } else if (pkCol->getType() == "VARCHAR") {
                    pkValue = std::make_shared<DataType_Varchar>(whereValue, pkCol->getMaxLen());
                } else if (pkCol->getType() == "DATE") {
                    pkValue = std::make_shared<DataType_Date>(whereValue);
                } else {
                    return "ERROR: Unsupported data type";
                }
            } catch (const std::exception& e) {
                std::cerr << "ERROR: Failed to convert value: " << e.what() << "\n";
                return "ERROR: Invalid value format - " + std::string(e.what());
            }
            
            auto row = table->selectByPrimaryKey(pkValue);
            if (row) {
                rows.push_back(row);
            }
            std::cout << "SELECT WHERE - found " << rows.size() << " rows\n";
        }
        
        if (rows.empty()) {
            std::cout << "No rows found\n";
            return "0 rows";
        }
        
        std::string result = std::to_string(rows.size()) + " row(s):\n";
        
        for (const auto& row : rows) {
            if (!row) {
                std::cerr << "ERROR: Null row pointer!\n";
                continue;
            }
            
            result += "Row " + std::to_string(row->getRowID()) + ": ";
            
            try {
                if (selectAll) {
                    auto data = row->getData();
                    for (const auto& [colName, value] : data) {
                        if (value) {
                            result += colName + "=" + value->toString() + " ";
                        } else {
                            result += colName + "=NULL ";
                        }
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
            } catch (const std::exception& e) {
                std::cerr << "ERROR: Exception while building result: " << e.what() << "\n";
                result += " [ERROR] ";
            }
            
            result += "\n";
        }
        
        std::cout << "SELECT response built successfully (" << result.length() << " bytes)\n";
        std::cout << "Response:\n" << result << "\n";
        
        return result;
        
    } catch (const std::exception& e) {
        std::cerr << "EXCEPTION in Select_Command::execute: " << e.what() << "\n";
        return "ERROR: Exception - " + std::string(e.what());
    } catch (...) {
        std::cerr << "UNKNOWN EXCEPTION in Select_Command::execute\n";
        return "ERROR: Unknown exception";
    }
}