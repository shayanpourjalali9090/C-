#include "ast.h"

ast_node::ast_node(){
    left=right=nullptr;
}

ast_node::ast_node(ast_node_type type, std::string value){
    this->type=type;
    this->value=value;
    left=right=nullptr;
}

ast_node::ast_node(ast_node_type type){
    this->type=type;
    left=right=nullptr;
}

ast::ast(){}

ast_node* ast::get(int i){
    return tree.at(i);
}

void ast::push(ast_node* node){
    tree.push_back(node);
}

std::string ast::to_string(){

    return "";
}