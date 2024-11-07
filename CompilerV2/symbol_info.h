#ifndef SYMBOL_INFO_H
#define SYMBOL_INFO_H

#include<bits/stdc++.h>
using namespace std;

class symbol_info
{
private:
    string name;
    string type;   

public:

    symbol_info(string name, string type)
    {
        this->name = name;
        this->type = type;
    }

    string get_name()
    {
        return name;
    }

    string get_type()
    {
        return type;
    }

    void set_name(string name)
    {
        this->name = name;
    }

    void set_type(string type)
    {
        this->type = type;
    }
};

#endif