#include "ast.h"

ast_node::ast_node()
    : left(nullptr), right(nullptr), else_node(nullptr), 
    true_node(nullptr), false_node(nullptr), condition(nullptr),
    table(nullptr), element_size_reference(nullptr), operand1_node(nullptr), 
    operand2_node(nullptr){}

ast_node::ast_node(ast_node_type type, std::string value)
    : type(type), value(value), left(nullptr), right(nullptr), 
    else_node(nullptr), true_node(nullptr), false_node(nullptr), 
    condition(nullptr), table(nullptr), element_size_reference(nullptr), 
    operand1_node(nullptr), operand2_node(nullptr){}

ast_node::ast_node(ast_node_type type)
    : type(type), left(nullptr), right(nullptr), 
    else_node(nullptr), true_node(nullptr), false_node(nullptr), 
    condition(nullptr), table(nullptr),element_size_reference(nullptr), 
    operand1_node(nullptr), operand2_node(nullptr){}

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

std::string node_type_to_string(ast_node_type type){
    std::string node_type_strs[]={
        "DECLARATION",
        "ARRAY_DECLARATION",
        "ARRAY",
        "EXPRESSION",
        "UNARY",
        "IDENTIFIER",
        "ARRAY_ACCESS",
        "FUNCTION_CALL",
        "NUMBER",
        "STRING",
        "BLOCK",
        "LOOP",
        "FUNCTION_DEFINITION",
        "RETURN",
        "BREAK",
        "CONTINUE",
        "IF",
        "ELIF",
        "ELSE",
        "ASM",
        "REGISTER",
    };

    return node_type_strs[(int)type];
}

void ast_node::print(int depth){
    std::string tabs;

    for (int i=0;i<depth;i++)tabs+=i%2==0?'|':'\t';

    printf("%sType: '%s'\n%sValue: '%s'\n",tabs.c_str(),node_type_to_string(type).c_str(),tabs.c_str(),value.c_str());

    if (!identifier.empty()){
        printf("%sIdentifier: '%s'\n",tabs.c_str(),identifier.c_str());
    }

    if (left!=nullptr){
        printf("%s--------Left:\n",tabs.c_str());
        left->print(depth+2);
    }
    if (right!=nullptr){
        printf("%s--------Right:\n",tabs.c_str());
        right->print(depth+2);
    }
    if (element_size_reference!=nullptr){
        printf("%s--------Element Size Reference:\n",tabs.c_str());
        element_size_reference->print(depth+2);
    }
    if (children.size()){
        printf("%s--------Children:\n",tabs.c_str());
        children.print(depth+2);
    }
    if (indexes.size()){
        printf("%s--------Indexes:\n",tabs.c_str());
        indexes.print(depth+2);
    }
    if (elements.size()){
        printf("%s--------Elements:\n",tabs.c_str());
        elements.print(depth+2);
    }
    if (args.size()){
        printf("%s--------Args:\n",tabs.c_str());
        args.print(depth+2);
    }
}

void ast::print(int depth){
    for (int i=0;i<tree.size();i++){
        ast_node* node=tree.at(i);
        if (node)
        node->print(depth);
    }

}