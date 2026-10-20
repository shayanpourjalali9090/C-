#pragma once

#include <string>

inline bool is_numeric(char c);

bool is_numeric(std::string &str);

bool is_operator(std::string &str);

bool is_delimiter(char c);

bool is_keyword(std::string &str);