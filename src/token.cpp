#include "token.h"
#include "utils.h"

token::token(){}

token::token(token_type type){
    this->type=type;
}

token::token(std::string str){
    type=is_numeric(str)?token_type::NUMBER
    :is_register(str)?token_type::REGISTER
    :is_keyword(str)?token_type::KEYWORD
    :token_type::IDENTIFIER;
    
    value=str;
}

token::token(token_type type, std::string value){
    this->type=type;
    this->value=value;
}

token token_list::operator++(){
    return advance();
}

token token_list::operator++(int){
    return advance();
}

std::string token_type_to_string(token_type type){
    switch (type){
        case token_type::DELIMITER:{
            return "Delimiter";
        }
        case token_type::IDENTIFIER:{
            return "Identifier";
        }
        case token_type::KEYWORD:{
            return "Keyword";
        }
        case token_type::NUMBER:{
            return "Number";
        }
        case token_type::OPERATOR:{
            return "Operator";
        }
        case token_type::STRING:{
            return "String";
        }
        default:return "";
    }
}

std::string token::to_string(){
    return "Type="+token_type_to_string(this->type)+", Value="+this->value;
    
}

token_list::token_list(){
    i=0;
}

token token_list::peek(){
    return at(i);
}

token token_list::advance(){
    return at(i++);
}

bool token_list::match(std::string value){
    return peek().value==value;
}

bool token_list::match(token_type type){
    return peek().type==type;
}

void token_list::expect(std::string value){
    token t=advance().value;

    if (t.type==token_type::END_OF_FILE){
        std::string errmsg="Expected '"+value+"'";

        emit_error(error::SYNTAX_ERROR,errmsg.c_str(),t.line_number);
        
    }
    else if(t.value!=value){
        std::string errmsg="Expected '"+value+"' but got '"+t.value+"' instead";

        emit_error(error::SYNTAX_ERROR,errmsg.c_str(),t.line_number);

    }

}

bool token_list::end(){
    return i>=tokens.size();
}

void token_list::push(token t){
    tokens.push_back(t);
}

int token_list::size(){
    return (int)tokens.size();
}

token token_list::at(int index){
    return index>=tokens.size()?token(token_type::END_OF_FILE):tokens.at(index);
}

token token_list::lookahead(){
    return at(i+1);
}

std::string token_list::to_string(){
    std::string ret="[ \n\n";
    int j;
    for (j=0;j<size()-1;++j){
        ret+="\t"+at(j).to_string()+"\n\n";
    }
    ret+="\t"+at(j).to_string()+"\n\n]";
    return ret;
}