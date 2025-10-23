#include "../include/DatabaseManager.h"
#include<iostream>
#include<fstream>

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
    return false;
}

bool DatabaseManager::isConnectionActive(int connectionId) const
{
    return false;
}

std::vector<ClientConnection> DatabaseManager::listActiveConnections() const
{
    return std::vector<ClientConnection>();
}

bool DatabaseManager::saveDatabase()
{
    return false;
}

bool DatabaseManager::loadDatabase()
{
    return false;
}

bool DatabaseManager::initializeDataDirectory()
{
    return false;
}

void DatabaseManager::closeAllConnections()
{
}
