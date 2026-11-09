#pragma once

#include "token.h"
#include <vector>
#include <string>

enum class symbol_type{
    FUNCTION,
    GLOBAL,
    LOCAL,

};

enum class symbol_table_type{
    GLOBAL,
    FUNCTION,
    BLOCK,

};

class symbol{
    public:

    symbol_type type;
    std::string name;
    int offset;

    symbol();

    symbol(symbol_type type);

    symbol(std::string name);

    symbol(std::string name, int offset);

    std::string get_address();

    inline std::string get_nasm_label();

    private:
};

class symbol_table{
    public:
    symbol_table_type type;
    std::vector<symbol*> table;
    symbol_table *parent;
    int cur_offset,locals_count=0;

    symbol_table(symbol_table_type type);

    symbol_table(symbol_table_type type, symbol_table* parent, int offset);

    symbol* lookup(std::string identifier);

    void inc_locals_count();

    void push(symbol* s);

    void push_params(std::vector<token> &params);

    private:
};