#pragma once

enum class symbol_type{
    FUNCTION,
    GLOBAL,
    LOCAL,
    UNKNOWN,

};

class symbol{
    public:

    std::string identifier;
    
    private:
};

class symbol_table{
    public:
    std::vector<symbol> table;
    symbol_table *parent;

    symbol_table();

    symbol_table(symbol_table *parent);

    symbol lookup(std::string identifier);

    private:

};