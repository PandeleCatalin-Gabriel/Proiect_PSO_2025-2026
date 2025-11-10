#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include "DatabaseManager.h"
#include <memory>
#include <string>
#include <thread>
#include <atomic>

class SocketServer {
private:
    int port;
    int serverSocket;
    std::atomic<bool> running;
    std::shared_ptr<DatabaseManager> dbManager;
    
    void handleClient(int clientSocket, std::string clientIP, int clientPort, int connId);
    std::string processCommand(const std::string& command, int connId);
    
public:
    SocketServer(int port, std::shared_ptr<DatabaseManager> manager);
    ~SocketServer();
    
    bool start();
    void waitForConnections();
    void stop();
};

#endif // SOCKETSERVER_H