#include <iostream>
#include "../include/DatabaseManager.h"
#include "../include/DataType_Int.h"
#include "../include/DataType_Varchar.h"

int main() {
    DatabaseManager manager("TestDB","./data");
    auto db =manager.getDatabase();
    db->createTable("Users");
    auto users = db->getTable("Users");
    users->addColumn("id", "INT", 0, true, false);
    users->addColumn("name", "VARCHAR", 50, false, false);
        
    std::map<std::string, std::shared_ptr<DataType_Interface>> user1;
    user1["id"] = std::make_shared<DataType_Int>(100);
    user1["name"] = std::make_shared<DataType_Varchar>("Ion", 50);
    users->insertRow(user1);
        
    std::map<std::string, std::shared_ptr<DataType_Interface>> user2;
    user2["id"] = std::make_shared<DataType_Int>(200);
    user2["name"] = std::make_shared<DataType_Varchar>("Ana", 50);
    users->insertRow(user2);

    
    return 0;
}