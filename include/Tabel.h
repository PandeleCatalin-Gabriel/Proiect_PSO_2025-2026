#pragma once
#include<string>
#include<vector>
#include"Column.h"
#include"Row.h"

class Tabel
{
private:
    std::string tableName;
    std::vector<Column> coloane;
    std::vector<std::shared_ptr<Row>> linii;
    
public:

};