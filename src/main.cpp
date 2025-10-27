#include <iostream>
#include "../include/DatabaseManager.h"
#include "../include/DataType_Int.h"
#include "../include/DataType_Varchar.h"
#include "../include/Insert_Command.h"
#include "../include/Delete_Command.h"
#include "../include/Select_Command.h"

int main() {
    DatabaseManager manager("TestDB", "./data");
    auto db = manager.getDatabase();
    
    db->createTable("Users");
    auto users = db->getTable("Users");
    users->addColumn("id", "INT", 0, true, false);
    users->addColumn("name", "VARCHAR", 50, false, false);
    
    std::cout << "Tabela are " << users->getRows().size() << " rânduri\n\n";
    
    Insert_Command insertCmd;
    insertCmd.parseCommand("INSERT INTO Users VALUES (100, 'Ion Popescu')");
    
    std::cout << "  Table: " << insertCmd.getTableName() << "\n";
    std::cout << "  Fields: ";
    for (const auto& field : insertCmd.getFields()) {
        std::cout << "[" << field << "] ";
    }
    std::cout << "\n";
    
    std::string result = insertCmd.execute(*db);
    std::cout << "Execute result: " << result << "\n";
    std::cout << "Tabela are acum " << users->getRows().size() << " rânduri\n\n";
    
    Insert_Command insertCmd2;
    insertCmd2.parseCommand("INSERT INTO Users VALUES (200, 'Ana Maria')");
    std::cout << insertCmd2.execute(*db) << "\n";
    std::cout << "Tabela are acum " << users->getRows().size() << " rânduri\n\n";

    Delete_Command deleteCmd;
    deleteCmd.parseCommand("DELETE FROM Users WHERE id = 100");
    
    std::cout << "  Table: " << deleteCmd.getTableName() << "\n";
    std::cout << "  Where: " << deleteCmd.getWhereColumn() 
    << " = " << deleteCmd.getWhereValue() << "\n";
    
    result = deleteCmd.execute(*db);
    std::cout << "Execute result: " << result << "\n";
    std::cout << "Tabela are acum " << users->getRows().size() << " rânduri\n\n";

    for (const auto& row : users->getRows()) {
        std::cout << "  ID=" << row->getValue("id")->toString()
        << ", Name=" << row->getValue("name")->toString() << "\n";
    }
    Select_Command selectCmd;
    selectCmd.parseCommand("SELECT * FROM Users WHERE id = 200");
    std::cout << "\n  Table: " << selectCmd.getTableName() << "\n";
    if (selectCmd.getHasWhere()) {
        std::cout << "  Where: " << selectCmd.getWhereColumn() 
        << " = " << selectCmd.getWhereValue() << "\n";
    }
    result = selectCmd.execute(*db);
    std::cout << "Execute result:\n" << result << "\n";
    
    
    return 0;
}