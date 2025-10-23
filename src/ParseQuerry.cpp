#include "../include/ParseQuerry.h"

std::string ParseQuerry::getCommnadType()
{
    return this->commandType;
}

void ParseQuerry::setCommandType(std::string type)
{
    this->commandType=type;
}
