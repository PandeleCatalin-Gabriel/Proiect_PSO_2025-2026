#include "Row.h"

int Row::getRowID() const
{
    return this->rowID;
}

std::map<std::string, std::shared_ptr<DataType_Interface>> Row::getData() const
{
    return this->data;
}

void Row::setRowID(int ID)
{
    this->rowID=ID;
}

std::shared_ptr<DataType_Interface> Row::getValue(const std::string &columnName)
{
    if(this->data.find(columnName)!= data.end())
    {
        return data[columnName];
    }
    return nullptr;
}

void Row::setValue(const std::string &columnName, std::shared_ptr<DataType_Interface> value)
{
    data[columnName]=value;
}

bool Row::hasColumn(const std::string &columnName) const
{
    return this->data.find(columnName)!= data.end();
}
