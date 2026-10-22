#pragma once

#include <string>
#include <vector>

enum class ast_node_type{
    DECLARATION,
    EXPRESSION,
    UNARY,
    IDENTIFIER,
    FUNCTION_CALL,
    NUMBER,
    BLOCK,
    LOOP,
    FUNCTION_DEFINITION,
    RETURN,
    IF,
    ASM,

};


class ast_node{
    public:

    ast_node();

    ~ast_node();

    ast_node(ast_node_type type, std::string value);

    ast_node(ast_node_type type);

    ast_node_type type;
    ast_node *left,*right,*condition,*true_node,*false_node;
    ast args,children;
    std::string value,identifier;
    std::vector<std::string> params;

    private:

};

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

    std::string to_string();

    private:

};