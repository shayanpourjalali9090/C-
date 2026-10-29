#pragma once

#include "symbol.h"
#include "token.h"

#include <string>
#include <vector>

enum class ast_node_type{
    DECLARATION,
    EXPRESSION,
    UNARY,
    IDENTIFIER,
    ARRAY_ACCESS,
    FUNCTION_CALL,
    NUMBER,
    STRING,
    BLOCK,
    LOOP,
    FUNCTION_DEFINITION,
    RETURN,
    BREAK,
    CONTINUE,
    IF,
    ELIF,
    ELSE,
    ASM,

};

std::string node_type_to_string(ast_node_type type);
class ast_node;

class ast{
    public:

    ast();

    ~ast();

    std::vector<ast_node*> tree;

    ast_node* get(int i);

    void clear();

    int size();

    bool empty();

    void push(ast_node *node);

    void print(int depth);

    private:

};

class ast_node{
    public:

    ast_node();

    ~ast_node();

    ast_node(ast_node_type type, std::string value);

    ast_node(ast_node_type type);

    void print(int depth);

    ast_node_type type;
    ast_node *left,*right,*condition,*true_node,*false_node,*else_node,*element_size_reference;
    ast args,children,elifs,indexes;
    std::string value,identifier;
    std::vector<token> params;
    symbol_table* table;

    private:

};