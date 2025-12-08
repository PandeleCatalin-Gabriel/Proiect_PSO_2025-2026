#include "../include/SocketServer.h"
#include "../include/Insert_Command.h"
#include "../include/Delete_Command.h"
#include "../include/Select_Command.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <mutex>


static std::mutex dbMutex;

SocketServer::SocketServer(int port, std::shared_ptr<DatabaseManager> manager) 
    : port(port), serverSocket(-1), running(false), dbManager(manager) {
}

SocketServer::~SocketServer() {
    stop();
}

bool SocketServer::start() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "ERROR: Cannot create socket\n";
        return false;
    }
    
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "ERROR: setsockopt failed\n";
        close(serverSocket);
        return false;
    }
    
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr)); 
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "ERROR: Bind failed on port " << port << " - " << strerror(errno) << "\n";
        close(serverSocket);
        return false;
    }
    
    if (listen(serverSocket, 10) < 0) {
        std::cerr << "ERROR: Listen failed - " << strerror(errno) << "\n";
        close(serverSocket);
        return false;
    }
    
    running = true;
    std::cout << "Server listening on port " << port << "\n";
    
    return true;
}

void SocketServer::waitForConnections() {
    while (running) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        memset(&clientAddr, 0, sizeof(clientAddr));
        
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket < 0) {
            if (running) {
                std::cerr << "ERROR: Accept failed - " << strerror(errno) << "\n";
            }
            continue;
        }
        
     
        struct timeval timeout;
        timeout.tv_sec = 30;  
        timeout.tv_usec = 0;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        int clientPort = ntohs(clientAddr.sin_port);
        
        std::cout << "New connection from " << clientIP << ":" << clientPort << "\n";
        
      
        int connId = -1;
        {
            std::lock_guard<std::mutex> lock(dbMutex);
            connId = dbManager->registerConnection(clientIP, clientPort);
        }
        std::cout << "Registered connection ID: " << connId << "\n";
        
      
        std::thread clientThread(&SocketServer::handleClient, this, clientSocket, 
                                std::string(clientIP), clientPort, connId);
        clientThread.detach();
    }
}

void SocketServer::handleClient(int clientSocket, std::string clientIP, int clientPort, int connId) {
    char buffer[4096];
    
    std::string welcomeMsg = "Connected to Database Server (Connection ID: " + 
                            std::to_string(connId) + ")\n";
    send(clientSocket, welcomeMsg.c_str(), welcomeMsg.length(), 0);
    
    while (running) {
        memset(buffer, 0, sizeof(buffer));
        
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesRead <= 0) {
            if (bytesRead == 0) {
                std::cout << "Client " << clientIP << ":" << clientPort << " disconnected normally\n";
            } else {
                std::cerr << "Error receiving from " << clientIP << ":" << clientPort 
                         << " - " << strerror(errno) << "\n";
            }
            break;
        }
        
       
        buffer[bytesRead] = '\0';
        std::string command(buffer);
        
 
        size_t pos;
        while ((pos = command.find('\n')) != std::string::npos) {
            command.erase(pos, 1);
        }
        while ((pos = command.find('\r')) != std::string::npos) {
            command.erase(pos, 1);
        }
        

        command.erase(0, command.find_first_not_of(" \t"));
        command.erase(command.find_last_not_of(" \t") + 1);
        
        if (command.empty()) {
            continue;
        }
        
        std::cout << "Received from " << clientIP << ":" << clientPort << ": " << command << "\n";
        

        if (command == "QUIT" || command == "EXIT") {
            std::string byeMsg = "Goodbye!\n";
            send(clientSocket, byeMsg.c_str(), byeMsg.length(), 0);
            break;
        }
        
        if (command == "PING") {
            std::string pongMsg = "PONG\n";
            send(clientSocket, pongMsg.c_str(), pongMsg.length(), 0);
            continue;
        }
        
  
        std::string response;
        {
            std::lock_guard<std::mutex> lock(dbMutex);
            response = processCommand(command, connId);
        }
        
    
        if (!response.empty() && response.back() != '\n') {
            response += "\n";
        }
        
        ssize_t sent = send(clientSocket, response.c_str(), response.length(), 0);
        if (sent < 0) {
            std::cerr << "Error sending to " << clientIP << ":" << clientPort 
                     << " - " << strerror(errno) << "\n";
            break;
        }
        
        std::cout << "Sent response to " << clientIP << ":" << clientPort 
                 << " (" << sent << " bytes)\n";
    }
    
    {
        std::lock_guard<std::mutex> lock(dbMutex);
        std::cout << "Cleaning up connection " << connId << "\n";
    }
    
    close(clientSocket);
    std::cout << "Connection " << connId << " from " << clientIP << ":" << clientPort << " closed\n";
}

std::string SocketServer::processCommand(const std::string& command, int connId) {
    try {
        auto db = dbManager->getDatabase();
        
        if (!db) {
            return "ERROR: Database not available";
        }
        
        std::string cmdUpper = command;
        std::transform(cmdUpper.begin(), cmdUpper.end(), cmdUpper.begin(), ::toupper);
        
        // ============ LIST TABLES ============
        if (cmdUpper == "LIST TABLES" || cmdUpper == "SHOW TABLES") {
            auto tables = db->listTables();
            std::string result = std::to_string(tables.size()) + " table(s):\n";
            for (const auto& tableName : tables) {
                result += tableName + "\n";
            }
            return result;
        }
        
        // ============ DESCRIBE table ============
        if (cmdUpper.substr(0, 8) == "DESCRIBE" || cmdUpper.substr(0, 4) == "DESC") {
            // Extract table name
            std::istringstream ss(command);
            std::string keyword, tableName;
            ss >> keyword >> tableName;
            
            if (tableName.empty()) {
                return "ERROR: Usage: DESCRIBE <table_name>";
            }
            
            auto table = db->getTable(tableName);
            if (!table) {
                return "ERROR: Table '" + tableName + "' not found";
            }
            
            std::string result = "Table: " + table->getTableName() + "\n";
            result += "Primary Key: " + (table->getPrimaryKeyColumn().empty() ? "(none)" : table->getPrimaryKeyColumn()) + "\n";
            result += "Columns:\n";
            
            for (const auto& col : table->getColumns()) {
                result += "  " + col.getName() + " " + col.getType();
                if (col.getMaxLen() > 0) {
                    result += "(" + std::to_string(col.getMaxLen()) + ")";
                }
                if (col.getIsPrimaryKey()) {
                    result += " PRIMARY KEY";
                }
                if (!col.getIsNullable()) {
                    result += " NOT NULL";
                }
                result += "\n";
            }
            
            result += "Row count: " + std::to_string(table->getRows().size()) + "\n";
            return result;
        }
        
        // ============ INSERT ============
        if (cmdUpper.substr(0, 6) == "INSERT") {
            Insert_Command cmd;
            cmd.parseCommand(command);
            std::string result = cmd.execute(*db);
            
            // Auto-save after insert
            dbManager->saveDatabase();
            
            return result;
        }
        
        // ============ DELETE ============
        else if (cmdUpper.substr(0, 6) == "DELETE") {
            Delete_Command cmd;
            cmd.parseCommand(command);
            std::string result = cmd.execute(*db);
            
            // Auto-save after delete
            dbManager->saveDatabase();
            
            return result;
        }
        
        // ============ SELECT ============
        else if (cmdUpper.substr(0, 6) == "SELECT") {
            Select_Command cmd;
            cmd.parseCommand(command);
            return cmd.execute(*db);
        }
        
        // ============ HELP ============
        else if (cmdUpper == "HELP") {
            return "Available commands:\n"
                   "  LIST TABLES - show all tables\n"
                   "  DESCRIBE <table> - show table structure\n"
                   "  INSERT INTO table VALUES (val1, val2, ...)\n"
                   "  DELETE FROM table WHERE column=value\n"
                   "  SELECT * FROM table [WHERE column=value]\n"
                   "  PING - test connection\n"
                   "  QUIT - disconnect\n";
        }
        else {
            return "ERROR: Unknown command '" + command + "'. Type HELP for available commands.";
        }
    }
    catch (const std::exception& e) {
        return std::string("ERROR: Exception - ") + e.what();
    }
    catch (...) {
        return "ERROR: Unknown exception occurred";
    }
}

void SocketServer::stop() {
    std::cout << "Stopping server...\n";
    running = false;
    
    if (serverSocket != -1) {
        shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
        serverSocket = -1;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::cout << "Server stopped\n";
}