#include "../include/DatabaseManager.h"
#include "../include/DataType_Int.h"
#include "../include/DataType_Varchar.h"
#include "../include/DataType_Date.h"
#include <nlohmann/json.hpp>
#include<iostream>
#include<fstream>
#include<filesystem>

using json = nlohmann::json;

DatabaseManager::DatabaseManager(const std::string &dbName, const std::string &dataDir)
    :dataDirectory(dataDir),nextConnectionId(1)
{
    database=std::make_shared<Database>(dbName,dataDir+"/"+ dbName+".db");
    dbFilePath=dataDirectory+"/"+dbName+".db";
    initializeDataDirectory();
}

DatabaseManager::~DatabaseManager()   
{
    closeAllConnections();
    saveDatabase();
}

size_t DatabaseManager::getActiveConnectionCount() const
{
    std::lock_guard<std::mutex> lock(connectionsMutex);
    return activeConnections.size();
}

int DatabaseManager::registerConnection(const std::string &clientIP, int clientPort)
{
    std::lock_guard<std::mutex> lock(connectionsMutex);

    ClientConnection NewConn;
    NewConn.clientIP=clientIP;
    NewConn.clientPort=clientPort;
    NewConn.connectionId=this->nextConnectionId++;
    NewConn.isActive=true;
    NewConn.threadId=std::this_thread::get_id();

    activeConnections[NewConn.connectionId]=NewConn;

    return NewConn.connectionId;
}

bool DatabaseManager::closeConnection(int connectionId)
{
    std::lock_guard<std::mutex>lock(connectionsMutex);
    auto it = activeConnections.find(connectionId);
    if (it == activeConnections.end()) {
        return false;
    }
    
    activeConnections.erase(it);
    return true;
}

bool DatabaseManager::isConnectionActive(int connectionId) const
{
    std::lock_guard<std::mutex>lock(connectionsMutex);

    auto it = activeConnections.find(connectionId);

    if(it != activeConnections.end() && it->second.isActive)
    {
        return true;
    }
    return false;
}

std::vector<ClientConnection> DatabaseManager::listActiveConnections() const
{
    std::lock_guard<std::mutex>lock(connectionsMutex);

    std::vector<ClientConnection> connections;
    for (const auto& [id, conn] : activeConnections) {
        connections.push_back(conn);
    }
    
    return connections;
}

bool DatabaseManager::saveDatabase()
{
    try {
        json dbJson;
        
        dbJson["database_name"] = database->getName();
        dbJson["tables"] = json::array();
        
        for (const auto& tableName : database->listTables()) {
            auto table = database->getTable(tableName);
            if (!table) continue;
            
            json tableJson;
            tableJson["table_name"] = table->getTableName();
            tableJson["primary_key_column"] = table->getPrimaryKeyColumn();
            tableJson["columns"] = json::array();
            tableJson["rows"] = json::array();
            
            for (const auto& col : table->getColumns()) {
                json colJson;
                colJson["name"] = col.getName();
                colJson["type"] = col.getType();
                colJson["max_length"] = col.getMaxLen();
                colJson["is_primary_key"] = col.getIsPrimaryKey();
                colJson["is_nullable"] = col.getIsNullable();
                tableJson["columns"].push_back(colJson);
            }
            
            for (const auto& row : table->getRows()) {
                json rowJson;
                rowJson["row_id"] = row->getRowID();
                rowJson["data"] = json::object();
                
                for (const auto& [colName, value] : row->getData()) {
                    json valueJson;
                    valueJson["type"] = value->getType();
                    valueJson["value"] = value->toString();
                    rowJson["data"][colName] = valueJson;
                }
                
                tableJson["rows"].push_back(rowJson);
            }
            
            dbJson["tables"].push_back(tableJson);
        }
        
     
        std::ofstream file(dbFilePath);
        if (!file.is_open()) {
            return false;
        }
        
        file << dbJson.dump(2); 
        file.close();
        
        return true;
        
    } catch (const std::exception& e) {
        return false;
    }
}

bool DatabaseManager::loadDatabase()
{
    try {
        if (!std::filesystem::exists(dbFilePath)) {
            return false; 
        }
        
        std::ifstream file(dbFilePath);
        if (!file.is_open()) {
            return false;
        }
        
        json dbJson;
        file >> dbJson;
        file.close();
        
        for (const auto& tableJson : dbJson["tables"]) {
            std::string tableName = tableJson["table_name"];
            
            if (!database->createTable(tableName)) {
                continue; 
            }
            
            auto table = database->getTable(tableName);
            if (!table) continue;
            
            for (const auto& colJson : tableJson["columns"]) {
                table->addColumn(
                    colJson["name"],
                    colJson["type"],
                    colJson["max_length"],
                    colJson["is_primary_key"],
                    colJson["is_nullable"]
                );
            }
            
            for (const auto& rowJson : tableJson["rows"]) {
                std::map<std::string, std::shared_ptr<DataType_Interface>> rowData;
                
                for (const auto& [colName, valueJson] : rowJson["data"].items()) {
                    std::string type = valueJson["type"];
                    std::string value = valueJson["value"];
                    
                    if (type == "INT") {
                        rowData[colName] = std::make_shared<DataType_Int>(std::stoi(value));
                    } else if (type == "VARCHAR") {
                        int maxLen = 255;
                        for (const auto& col : table->getColumns()) {
                            if (col.getName() == colName) {
                                maxLen = col.getMaxLen();
                                break;
                            }
                        }
                        rowData[colName] = std::make_shared<DataType_Varchar>(value, maxLen);
                    } else if (type == "DATE") {
                        rowData[colName] = std::make_shared<DataType_Date>(value);
                    }
                }
                
                table->insertRow(rowData);
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        return false;
    }
}

bool DatabaseManager::initializeDataDirectory()
{
    try
    {
         if (!std::filesystem::exists(dataDirectory)) {
            std::filesystem::create_directories(dataDirectory);
        }
        if (!std::filesystem::is_directory(dataDirectory)) {
            return false;
        }
        return true;
    }
    catch(const std::exception& e)
    {
        return false;
    }
}

void DatabaseManager::closeAllConnections()
{
    std::lock_guard<std::mutex> lock(connectionsMutex);
    activeConnections.clear();
}
