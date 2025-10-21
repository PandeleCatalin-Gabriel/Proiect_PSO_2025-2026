#include "../include/Database.h"
#include<iostream>

Database::~Database()
{
    tables.clear();
    std::cout<<"Database "<<this->dbName<< "destroyed\n";
}

bool Database::createTable(const std::string& tableName) {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    if (tables.find(tableName) != tables.end()) {
        std::cout << "EROARE: Tabelul '" << tableName << "' există deja!\n";
        return false;
    }
    
    auto newTable = std::make_shared<Tabel>(tableName);
    tables[tableName] = newTable;

    return true;
}

bool Database::dropTable(const std::string& tableName) {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    auto it = tables.find(tableName);
    if (it == tables.end()) {
        return false;
    }
    
    tables.erase(it);
    return true;
}

std::shared_ptr<Tabel> Database::getTable(const std::string& tableName) {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    auto it = tables.find(tableName);
    if (it != tables.end()) {
        return it->second;
    }
    return nullptr;
}

bool Database::hasTable(const std::string& tableName) const {
    std::lock_guard<std::mutex> lock(dbMutex);
    return tables.find(tableName) != tables.end();
}

std::vector<std::string> Database::listTables() const {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    std::vector<std::string> tableNames;
    for (const auto& pair : tables) {
        tableNames.push_back(pair.first);
    }
    return tableNames;
}

bool Database::saveToFile() {
    std::lock_guard<std::mutex> lock(dbMutex);
    // TODO: Implementare JSON/Binary serialization
    return false;
}

// Load from file (implementare simplă - pentru mai târziu)
bool Database::loadFromFile() {
    std::lock_guard<std::mutex> lock(dbMutex);
    // TODO: Implementare JSON/Binary deserialization
    return false;
}