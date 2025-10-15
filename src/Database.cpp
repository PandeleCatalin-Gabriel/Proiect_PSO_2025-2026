#include "../include/Database.h"
#include<iostream>

Database::~Database()
{
    tables.clear();
    std::cout<<"Database "<<this->dbName<< "destroyed\n";
}