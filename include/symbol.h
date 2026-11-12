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
    STRUCT,

};

class symbol_table;

class symbol{
    public:

    symbol_type type;
    std::string name;
    int offset;
    int size;
    std::vector<int> array_sizes;
    int element_size;
    int base_offset;
    class symbol_table* parent_struct;

    bool is_array=false;

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
    std::vector<symbol_table*> structs;
    std::vector<symbol*> struct_fields;
    symbol_table* parent;
    int cur_offset,locals_count=0;
    int total_size=0;
    std::string name;

    symbol_table(symbol_table_type type);

    symbol_table(symbol_table_type type, symbol_table* parent, int offset);

    symbol* lookup(std::string identifier);

    void inc_locals_count();

    void increase_total_size(int size);

    void push(symbol* s);

    void push_struct(symbol_table* s);

    symbol_table* lookup_struct(std::string name);

    symbol* lookup_field(std::string field_name);

    void push_params(std::vector<token> &params);

    private:
};