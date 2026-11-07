#include <string>
#include <vector>
#include <cstdlib>

#include "symbol.h"
#include "utils.h"
#include "ast.h"

symbol::symbol(){}

symbol::symbol(symbol_type type)
    : type(type){}

symbol::symbol(std::string name)
    : name(name){}

symbol::symbol(std::string name, int offset)
    : name(name), offset(offset){}

std::string symbol::get_address(){
    if (type==symbol_type::GLOBAL || type==symbol_type::FUNCTION){
        return get_nasm_label();
    }

    std::string address="RBP";

    address+=offset>=0?"+":"-";

    std::string absoffset=std::to_string(std::abs(offset));

    address+=absoffset;

    return address;

}

symbol_table::symbol_table(symbol_table_type type)
    : type(type), parent(nullptr){}

symbol_table::symbol_table(symbol_table_type type, symbol_table* parent, int offset)
    : type(type), parent(parent), cur_offset(offset){}

symbol* symbol_table::lookup(std::string &identifier){
    for (int i=0;i<table.size();++i){
        if (table.at(i)->name==identifier)
            return table.at(i);

    }
    if (parent!=nullptr)
        return parent->lookup(identifier);

    return nullptr;
}

void symbol_table::inc_locals_count(){
    if (type==symbol_table_type::GLOBAL)return;

    if (type==symbol_table_type::FUNCTION){
        locals_count++;

    }
    else{
        parent->inc_locals_count();
    }
    
}

void symbol_table::push(symbol* s){
    table.push_back(s);
    if (s->type==symbol_type::LOCAL)
        inc_locals_count();
}

inline std::string symbol::get_nasm_label(){
    return '?'+name;
}

void symbol_table::push_params(std::vector<token> &params){
    int offset=16;
    
    for (int i=0;i<params.size();i++){

        symbol* s=new symbol(symbol_type::LOCAL);
        s->offset=offset;
        s->name=params[i].value;
        
        offset+=8;

        table.push_back(s);

    }
    
}