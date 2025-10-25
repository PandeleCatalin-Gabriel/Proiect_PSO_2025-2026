#include "../include/Insert_Command.h"
#include<sstream>
#include<string>

void Insert_Command::parseCommand(std::string command)
{
    std::istringstream ss(command);
    std::string word;
    
    ss>>word;
    if(word=="INSERT")
    {
        ss>>word;
        if(word=="INTO")
        {
            ss>>word;
            this->tableName=word;
            ss>>word;
            if(word=="VALUES")
            {
                std::string rest;
                std::getline(ss, rest);
                size_t start=rest.find('(');
                size_t end = rest.find(')');

                if (start != std::string::npos && end != std::string::npos)
                {
                    std::string valuesStr = rest.substr(start + 1, end - start - 1);
                    std::istringstream valuesStream(valuesStr);
                    std::string value;

                    while (std::getline(valuesStream, value, ','))
                    {
                        value.erase(0, value.find_first_not_of(" \t"));
                        value.erase(value.find_last_not_of(" \t") + 1);

                        if (!value.empty() && value.front() == '\'' && value.back() == '\'')
                        {
                             value = value.substr(1, value.length() - 2);
                        }
                        this->fields.push_back(value);
                    }

                }
            }
        }
        
        
    }
}

std::string Insert_Command::execute(Database& db) {
   
    return "INSERT command - TODO";
}