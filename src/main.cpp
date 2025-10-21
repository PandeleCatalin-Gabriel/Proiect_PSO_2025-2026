#include <iostream>
#include <memory>
#include "../include/Tabel.h"
#include "../include/DataType_Int.h"
#include "../include/DataType_Varchar.h"
#include "../include/DataType_Date.h"
#include "../include/Database.h"

int main() {
    
    Database db("TestDB");
    db.createTable("Angajati");
    auto tabel = db.getTable("Angajati");
    return 0;
}