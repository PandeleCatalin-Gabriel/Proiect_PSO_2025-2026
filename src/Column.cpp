#include "Column.h"

std::string Column::getName() const
{
    return this->name;
}

std::string Column::getType() const
{
    return this->type;
}

int Column::getMaxLen() const
{
    return this->maxLength;
}

bool Column::getIsPrimaryKey() const
{
    return this->isPrimaryKey;
}

bool Column::getIsNullable() const
{
    return this->isNull;
}

bool Column::getHasIndex() const
{
    return this->hasIndex;
}
