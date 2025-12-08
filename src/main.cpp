#include <iostream>
#include <signal.h>
#include "../include/DatabaseManager.h"
#include "../include/SocketServer.h"

std::shared_ptr<SocketServer> server;

void signalHandler(int signum) {
    std::cout << "\nShutting down server...\n";
    
    if (server) {
        server->stop();
        
        // Save database before exit
        std::cout << "Saving database...\n";
        // Note: DatabaseManager saves in destructor, but we can also save explicitly
    }
    
    exit(0);
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    std::cout << "=== Database Server Starting ===\n";
    
    auto manager = std::make_shared<DatabaseManager>("MainDB", "./data");
    
    // Try to load existing database
    if (manager->loadDatabase()) {
        std::cout << "✓ Existing database loaded successfully\n";
        auto db = manager->getDatabase();
        std::cout << "  Tables found: ";
        for (const auto& tableName : db->listTables()) {
            std::cout << tableName << " ";
        }
        std::cout << "\n";
    } else {
        std::cout << "No existing database found. Creating default schema...\n";
        
        auto db = manager->getDatabase();
        
        // Create Users table with more fields
        db->createTable("Users");
        auto users = db->getTable("Users");
        users->addColumn("id", "INT", 0, true, false);
        users->addColumn("name", "VARCHAR", 50, false, false);
        users->addColumn("email", "VARCHAR", 100, false, true);
        users->addColumn("created", "DATE", 0, false, true);
        
        std::cout << "✓ Default schema created\n";
        
        // Save initial database
        if (manager->saveDatabase()) {
            std::cout << "✓ Initial database saved to disk\n";
        }
    }
    
    server = std::make_shared<SocketServer>(5432, manager);
    
    if (!server->start()) {
        std::cerr << "Failed to start server\n";
        return 1;
    }
    
    std::cout << "\nServer ready. Waiting for connections...\n";
    std::cout << "Database will be saved automatically on changes\n";
    std::cout << "Press Ctrl+C to stop\n\n";
    
    server->waitForConnections();
    
    // Save on normal exit too
    std::cout << "\nSaving database...\n";
    manager->saveDatabase();
    
    return 0;
}