#pragma once
#include<iostream>
#include"Command_Interface.h"
#include"Delete_Command.h"
#include"Insert_Command.h"
#include"Select_Command.h"
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