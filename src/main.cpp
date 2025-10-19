#include <iostream>
#include <memory>
#include "../include/Tabel.h"
#include "../include/DataType_Int.h"
#include "../include/DataType_Varchar.h"
#include "../include/DataType_Date.h"

int main() {
    std::cout << "Server DB - Test functional\n";
    
    // Test rapid
    Tabel users("Users");
    users.addColumn("id", "INT", 0, true, false);
    users.addColumn("name", "VARCHAR", 50, false, false);
    
    std::map<std::string, std::shared_ptr<DataType_Interface>> user;
    user["id"] = std::make_shared<DataType_Int>(1);
    user["name"] = std::make_shared<DataType_Varchar>("Test", 50);
    
    if (users.insertRow(user)) {
        auto found = users.selectByPrimaryKey(std::make_shared<DataType_Int>(1));
        if (found) {
            std::cout << "Test passed: Insert + Select functional!\n";
        }
    }
    
    return 0;
}