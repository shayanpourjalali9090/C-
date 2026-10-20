#pragma once

#include <string>

enum class error{
    LEXICAL_ERROR,
    SYNTAX_ERROR,
    SEMANTIC_ERROR,
    CODE_GENERATION_ERROR,
};

inline bool is_numeric(char c);

bool is_numeric(std::string &str);

bool is_operator(std::string &str);

bool is_register(std::string &str);

bool is_delimiter(char c);

bool is_keyword(std::string &str);

void emit_error(error err,const char *msg);