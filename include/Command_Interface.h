#pragma once
#include<iostream>
#include"Database.h"

class Command_Interface
{
public:
    //virtual ~Command_Interface();
    
    virtual std::string execute(Database& db)=0;
};