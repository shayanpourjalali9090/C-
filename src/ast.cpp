#include "ast.h"

ast_node::ast_node()
    : left(nullptr), right(nullptr), else_node(nullptr), 
    true_node(nullptr), false_node(nullptr), condition(nullptr){}

ast_node::ast_node(ast_node_type type, std::string value)
    : type(type), value(value), left(nullptr), right(nullptr), 
    else_node(nullptr), true_node(nullptr), false_node(nullptr), 
    condition(nullptr){}

ast_node::ast_node(ast_node_type type)
    : type(type), left(nullptr), right(nullptr), 
    else_node(nullptr), true_node(nullptr), false_node(nullptr), 
    condition(nullptr){}

ast_node::~ast_node(){
    delete left;
    delete right;
    delete condition;
    delete true_node;
    delete false_node;
    delete else_node;
    
}

ast::ast(){}

ast::~ast(){
    int sz=size();
    for (int i=0;i<sz;++i){
        delete get(i);
    }
    clear();
}

void ast::clear(){
    tree.clear();
}

bool ast::empty(){
    return tree.empty();
}

int ast::size(){
    return (int)tree.size();
}

ast_node* ast::get(int i){
    return tree.at(i);
}

void ast::push(ast_node* node){
    tree.push_back(node);
}

std::string ast::to_string(){

    return "";
}