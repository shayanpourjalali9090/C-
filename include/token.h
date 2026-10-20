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

};

class token{
    public:
    token_type type;
    std::string value;

    token();

    token(std::string str);

    token(token_type type,std::string value);

    std::string to_string();

    private:

};

class token_list{
    public:
    int i;
    std::vector<token> tokens;

    token_list();

    token peek();

    token advance();

    bool end();

    bool expect(std::string value);

    void push(token t);

    private:

};