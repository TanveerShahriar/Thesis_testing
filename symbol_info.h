#ifndef SYMBOL_INFO_H
#define SYMBOL_INFO_H

#include<bits/stdc++.h>
using namespace std;

class symbol_info
{
private:
    string name;
    string type;

    string symbol_type; // variable, array, or function
    string return_type;
    vector<string> param_types; // Parameter types if it's a function
    int array_size;    // Array size if it's an array

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

    string get_symbol_type()
    {
        return symbol_type;
    }

    string get_return_type()
    {
        return return_type;
    }

    int get_array_size()
    {
        return array_size;
    }

    void add_param_type(string param_type)
    {
        param_types.push_back(param_type);
    }

    void set_name(string name)
    {
        this->name = name;
    }

    void set_type(string type)
    {
        this->type = type;
    }

    void set_symbol_type(string symbol_type)
    {
        this->symbol_type = symbol_type;
    }

    void set_return_type(string return_type)
    {
        this->return_type = return_type;
    }

    void set_array_size(int size)
    {
        this->array_size = size;
        this->symbol_type = "array"; // Set symbol type to array if array size is set
    }

    vector<string> get_param_types()
    {
        return param_types;
    }
};

#endif