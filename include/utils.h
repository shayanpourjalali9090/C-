#pragma once

#include <string>

enum class error{
    LEXICAL_ERROR,
    SYNTAX_ERROR,
    SEMANTIC_ERROR,
    CODE_GENERATION_ERROR,
};

bool is_reg8(std::string str);

bool is_reg16(std::string str);

bool is_reg32(std::string str);

bool is_reg64(std::string str);

bool is_asm_keyword(std::string &str);

void is_hex(std::string str);

void is_binary(std::string str);

inline bool is_numeric(char c);

bool is_numeric(std::string &str);

bool is_operator(std::string &str);

bool is_register(std::string str);

bool is_delimiter(char c);

bool is_keyword(std::string &str);

void emit_error(error err, std::string msg, int line_number);

void emit_error(error err, std::string msg);

std::string sformat(std::string str, ...);

bool is_binary_operator(std::string str);

int get_asm_instruction_length(std::string &keyword);