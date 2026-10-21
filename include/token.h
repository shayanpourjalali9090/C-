#pragma once

#include <vector>
#include <string>

enum class token_type{
    STRING,
    IDENTIFIER,
    KEYWORD,
    OPERATOR,
    DELIMITER,
    NUMBER,
    REGISTER,
    END_OF_FILE,

};

std::string token_type_to_string(token_type type);

class token{
    public:

    token_type type;
    std::string value;
    int line_number;

    token();

    token(std::string str);

    token(token_type type,std::string value);

    token(token_type type);

    std::string to_string();

    private:

};

class token_list{
    public:

    int i;
    
    std::vector<token> tokens;

    token_list();

    token peek();

    bool match(std::string value);

    bool match(token_type type);

    token at(int i);

    token lookahead();

    token advance();

    bool end();

    void expect(std::string value);

    token operator++();

    token operator++(int);

    void push(token t);

    std::string to_string();

    int size();

    private:

};