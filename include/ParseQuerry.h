#pragma once
#include<iostream>
#include"Commnad_Interface.h"
#include<string.h>

class ParseQuerry
{
private:
    std::string commandType;

public:
    ParseQuerry():commandType(nullptr){}

    //getter
    std::string getCommnadType();

    //setter
    void setCommandType(std::string type);
};