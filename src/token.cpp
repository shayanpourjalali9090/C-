#include "token.h"
#include "utils.h"

token::token(){}

token::token(std::string str){
    type=is_numeric(str)?token_type::NUMBER:is_keyword(str)?token_type::KEYWORD:token_type::IDENTIFIER;
    value=str;
}

token::token(token_type type, std::string value){
    this->type=type;
    this->value=value;
}

std::string token::to_string(){
    return this->value;
    
}

token_list::token_list(){
    i=0;
}

token token_list::peek(){
    return tokens.at(i);
}

token token_list::advance(){
    return tokens.at(i++);
}

bool token_list::expect(std::string value){
    return advance().value==value;
}

bool token_list::end(){
    return i==tokens.size();
}

void token_list::push(token t){
    tokens.push_back(t);
}