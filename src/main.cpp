#include <iostream>
#include <signal.h>
#include "../include/DatabaseManager.h"
#include "../include/SocketServer.h"

std::shared_ptr<SocketServer> server;

void signalHandler(int signum) {
    std::cout << "\nShutting down server...\n";
    if (server) {
        server->stop();
    }
    exit(0);
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    std::cout << "=== Database Server Starting ===\n";
    
    auto manager = std::make_shared<DatabaseManager>("MainDB", "./data");
    
    auto db = manager->getDatabase();
    db->createTable("Users");
    auto users = db->getTable("Users");
    users->addColumn("id", "INT", 0, true, false);
    users->addColumn("name", "VARCHAR", 50, false, false);
    
    std::cout << "Database initialized\n";
    
    server = std::make_shared<SocketServer>(5432, manager);
    
    if (!server->start()) {
        std::cerr << "Failed to start server\n";
        return 1;
    }
    
    std::cout << "Server ready. Waiting for connections...\n";
    std::cout << "Press Ctrl+C to stop\n\n";
    
    server->waitForConnections();
    
    return 0;
}