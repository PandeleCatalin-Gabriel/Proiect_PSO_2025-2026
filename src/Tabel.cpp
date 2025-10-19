#include "../include/Tabel.h"
#include <iostream>
#include <iomanip>

Tabel::Tabel(const std::string& name) 
    : tableName(name), nextRowId(1), primaryKeyColumn("") {
    std::cout << "Tabel '" << tableName << "' creat.\n";
}

Tabel::~Tabel() {
    linii.clear();
    coloane.clear();
    primaryIndex.reset();
    std::cout << "Tabel '" << tableName << "' distrus.\n";
}

void Tabel::addColumn(const std::string& name, const std::string& type, int maxLen, bool isPK, bool nullable) {
    Column col(name, type, maxLen, isPK, nullable);
    
    if (isPK) {
        if (!primaryKeyColumn.empty()) {
            std::cout << "EROARE: Tabelul are deja PRIMARY KEY pe coloana '" 
            << primaryKeyColumn << "'!\n";
            return;
        }
        primaryKeyColumn = name;
        col.setHasIndex(true);
        
        primaryIndex = std::make_unique<BTree<std::shared_ptr<DataType_Interface>, std::shared_ptr<Row>>>(3);
        std::cout << "PRIMARY KEY setat pe coloana '" << name << "'. B-tree creat.\n";
    }
    
    coloane.push_back(col);
    std::cout << "Coloană adăugată: " << name << " (" << type;
    if (maxLen > 0) std::cout << "(" << maxLen << ")";
    std::cout << ")";
    if (isPK) std::cout << " PRIMARY KEY";
    if (!nullable) std::cout << " NOT NULL";
    std::cout << "\n";
}

bool Tabel::hasColumn(const std::string& columnName) const {
    for (const auto& col : coloane) {
        if (col.getName() == columnName) {
            return true;
        }
    }
    return false;
}

const Column* Tabel::getColumn(const std::string& columnName) const {
    for (const auto& col : coloane) {
        if (col.getName() == columnName) {
            return &col;
        }
    }
    return nullptr;
}

bool Tabel::validateRow(const std::map<std::string, std::shared_ptr<DataType_Interface>>& data) const {
    
    if (!primaryKeyColumn.empty() && data.find(primaryKeyColumn) == data.end()) {
        std::cout << "EROARE: PRIMARY KEY '" << primaryKeyColumn << "' lipseste!\n";
        return false;
    }

    for (const auto& col : coloane) {
        if (!col.getIsNullable() && data.find(col.getName()) == data.end()) {
            std::cout << "EROARE: Coloana '" << col.getName() << "' este NOT NULL si lipseste!\n";
            return false;
        }
    }

    for (const auto& [colName, value] : data) {
        if (!hasColumn(colName)) {
            std::cout << "EROARE: Coloana '" << colName << "' nu exista în tabel!\n";
            return false;
        }
    }

    for (const auto& [colName, value] : data) {
        const Column* col = getColumn(colName);
        if (col && col->getType() != value->getType()) {
            std::cout << "EROARE: Coloana '" << colName << "' este " << col->getType() 
                      << " dar ai dat " << value->getType() << "!\n";
            return false;
        }
    }
    
    return true;
}

bool Tabel::insertRow(const std::map<std::string, std::shared_ptr<DataType_Interface>>& data) {
    if (!validateRow(data)) {
        std::cout << "Inserare eșuată - validare eronată.\n";
        return false;
    }

    auto newRow = std::make_shared<Row>(nextRowId++);
    for (const auto& [colName, value] : data) {
        newRow->setValue(colName, value);
    }

    linii.push_back(newRow);

    if (primaryIndex && !primaryKeyColumn.empty()) {
        auto pkValue = newRow->getValue(primaryKeyColumn);
        primaryIndex->insert(pkValue, newRow);
        std::cout << "Rand inserat cu PRIMARY KEY = " << pkValue->toString() 
                  << " (rowId intern = " << newRow->getRowID() << ")\n";
    } else {
        std::cout << "Rand inserat (rowId = " << newRow->getRowID() << ")\n";
    }
    
    return true;
}

std::shared_ptr<Row> Tabel::selectByPrimaryKey(const std::shared_ptr<DataType_Interface>& key) {
    if (!primaryIndex) {
        std::cout << "EROARE: Niciun PRIMARY KEY definit!\n";
        return nullptr;
    }
    
    auto row = primaryIndex->search(key);
    if (row) {
        std::cout << "Rand gasit cu PRIMARY KEY = " << key->toString() << "\n";
    } else {
        std::cout << "Niciun rand cu PRIMARY KEY = " << key->toString() << "\n";
    }
    
    return row;
}

std::vector<std::shared_ptr<Row>> Tabel::selectAll() {
    return linii;
}

bool Tabel::updateRow(const std::shared_ptr<DataType_Interface>& primaryKey, const std::string& columnName, const std::shared_ptr<DataType_Interface>& newValue) {

    auto row = selectByPrimaryKey(primaryKey);
    if (!row) {
        std::cout << "UPDATE esuat - rand negasit.\n";
        return false;
    }

    if (!hasColumn(columnName)) {
        std::cout << "UPDATE eșuat - coloana '" << columnName << "' nu există!\n";
        return false;
    }

    row->setValue(columnName, newValue);
    std::cout << "Rand actualizat: " << columnName << " = " << newValue->toString() << "\n";
    
    return true;
}

// DELETE
bool Tabel::deleteRow(const std::shared_ptr<DataType_Interface>& primaryKey) {

    auto row = selectByPrimaryKey(primaryKey);
    if (!row) {
        std::cout << "DELETE esuat - rand negasit.\n";
        return false;
    }
    
    linii.erase(std::remove(linii.begin(), linii.end(), row), linii.end());
    
    if (primaryIndex) {
        primaryIndex->remove(primaryKey);
    }
    
    std::cout << "Rând șters cu PRIMARY KEY = " << primaryKey->toString() << "\n";
    return true;
}

