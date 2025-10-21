#pragma once
#include<string>
#include<map>
#include<memory>
#include <mutex>
#include"Tabel.h"

class Database
{
private:
    std::map<std::string,std::shared_ptr<Tabel>> tables;
    std::string dbName;
    std::string filepath;
    mutable std::mutex dbMutex;
public:
    Database(const std::string &name="MyDatabase",const std::string &filepath = "database.json")
    : dbName(name), filepath(filepath) {}

    ~Database();

    //getteri
    std::string getName() const { return dbName; }
    std::string getFilepath() const { return filepath; }
    size_t getTableCount() const { return tables.size(); }
    
    //manipulare tabele
    bool createTable(const std::string& tableName);
    bool dropTable(const std::string& tableName);
    std::shared_ptr<Tabel> getTable(const std::string& tableName);
    bool hasTable(const std::string& tableName) const;
    std::vector<std::string> listTables() const;
    
    //mecanism persitenta
    bool saveToFile();
    bool loadFromFile();

};