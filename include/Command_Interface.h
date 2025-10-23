#pragma once
#include<iostream>
#include"Database.h"

class Command_Interface
{
public:
    virtual ~Command_Interface();
    virtual void execCommand(Database db)=0;
};