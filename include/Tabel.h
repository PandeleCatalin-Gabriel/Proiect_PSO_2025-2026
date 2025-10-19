#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <iostream>
#include "Column.h"
#include "Row.h"
#include "BTree.h"
#include "DataType_Interface.h"

class Tabel
{
private:
    std::string tableName;
    std::vector<Column> coloane;
    std::vector<std::shared_ptr<Row>> linii;
    
    // Btree de PK
    std::unique_ptr<BTree<std::shared_ptr<DataType_Interface>, std::shared_ptr<Row>>> primaryIndex;
    
    // metadata
    int nextRowId;
    std::string primaryKeyColumn;
public:
    Tabel(const std::string & name);
    ~Tabel();

    //getteri
    std::string getTableName()const{return this->tableName;}
    const std::vector<Column>&getColumns()const {return this->coloane; }
    const std::vector<std::shared_ptr<Row>>& getRows() const { return linii; }
    std::string getPrimaryKeyColumn() const { return primaryKeyColumn; }

    void addColumn(const std::string& name, const std::string& type, 
    int maxLen = 0, bool isPK = false, bool nullable = true);
    bool hasColumn(const std::string& columnName) const;
    const Column* getColumn(const std::string& columnName) const;
    
    bool insertRow(const std::map<std::string, std::shared_ptr<DataType_Interface>>& data);
    std::shared_ptr<Row> selectByPrimaryKey(const std::shared_ptr<DataType_Interface>& key);
    std::vector<std::shared_ptr<Row>> selectAll();
    bool updateRow(const std::shared_ptr<DataType_Interface>& primaryKey, 
    const std::string& columnName, 
    const std::shared_ptr<DataType_Interface>& newValue);
    bool deleteRow(const std::shared_ptr<DataType_Interface>& primaryKey);
    
    // Validare
    bool validateRow(const std::map<std::string, std::shared_ptr<DataType_Interface>>& data) const;

};