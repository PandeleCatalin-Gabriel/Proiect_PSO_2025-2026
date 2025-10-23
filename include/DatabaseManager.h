#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <memory>
#include <thread>
#include "Database.h"

struct ClientConnection {
    int connectionId;
    std::string clientIP;
    int clientPort;
    bool isActive;
    std::thread::id threadId;
};

class DatabaseManager
{
private:
    std::shared_ptr<Database> database;
    
    std::map<int, ClientConnection> activeConnections;
    int nextConnectionId;
    mutable std::mutex connectionsMutex;
    

    std::string dataDirectory;
    std::string dbFilePath;
    
public:
    DatabaseManager(const std::string& dbName = "MainDB", 
    const std::string& dataDir = "/opt/db_manager");
    
    ~DatabaseManager();
    
    //getteri
    std::shared_ptr<Database> getDatabase() { return database; }
    std::string getDataDirectory() const { return dataDirectory; }
    size_t getActiveConnectionCount() const;
    
    // conexiuni client
    int registerConnection(const std::string& clientIP, int clientPort);
    bool closeConnection(int connectionId);
    bool isConnectionActive(int connectionId) const;
    std::vector<ClientConnection> listActiveConnections() const;
    
    //manipulare date
    bool saveDatabase();
    bool loadDatabase();
    bool initializeDataDirectory();
    
    //cleanup
    void closeAllConnections();
};