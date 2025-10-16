#pragma once
#include<string>
#include<map>
#include<memory>
#include"Tabel.h"

class Database
{
private:
    std::map<std::string,std::shared_ptr<Tabel>> tables;
    std::string dbName;
    std::string filepath;
public:
    Database(const std::string &name="MyDatabase",const std::string &filepath = "database.json")
    : dbName(name), filepath(filepath) {}

    ~Database();


};