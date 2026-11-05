#include <iostream>

#include <cstdarg>

#include "utils.h"

const char delimiters[]={
    '+','-','*','/','^',' ','\n','%','=',':',';',
    '$','\"','\'','!','|','&','(',')','[',']','{',
    '}','~','@','>','<','?','.',',',
};

const std::string operators[]={
    ">",">=","<","<=","+","++","-","--","/","*","^",
    "?","~","%","+=","-=","*=","/=","%=","^=","|=",
    "&=","==","=","!=","!",">>","<<",">>=","<<=","@",
    "||","&&","^","&","|",
};

const std::string keywords[]={
    "let","while","if","elif","else","func",
    "return","break","continue","true","false",
    "MOV","ADD","SUB","IDIV","IMUL","JMP","JG",
    "JGE","JL","JLE","JNE","JE","CALL","RET",
    "CMP", "SHR", "SHL", "INC","DEC","SYSCALL",

};

const std::string registers[]={
    "RAX","RBX","RCX","RDX","RDI","RSI",
    "RBP","RSP","R8","R9","R10","R11",
    "R12","R13","R14","R15"
};

const std::string binary_operators[]={
    "+","-","*","/","%","^","&","|",
    "==","!=",">>","<<",">",">=","<","<=",

};

bool is_binary_operator(std::string str){
    if (str.empty())return false;
    
    for (std::string op:binary_operators){
        if (op==str)return true;
    }
    
    return false;

}

bool is_register(std::string &str){
    for (std::string reg:registers){
        if (str==reg)return true;
    }
    return false;
}

bool is_delimiter(char c){
    for (char delimiter:delimiters){
        if (delimiter==c)return true;
    }
    return false;
}

inline bool is_numeric(char c){
    return c>='0' && c<='9';
}

bool is_numeric(std::string &str){
    for (char c:str){
        if (!is_numeric(c))return false;
    }

    return true;
}

bool is_operator(std::string &str){
    for (std::string op:operators){
        if (str==op)return true;
    }
    
    return false;
}

bool is_keyword(std::string &str){
    for (std::string keyword:keywords){
        if (str==keyword)return true;
    }

    return false;
}

void emit_error(error err, std::string msg, int line_number){
    switch (err){
        case error::LEXICAL_ERROR:{
            std::cout << "Lexical Error: ";
            break;
        }
        case error::SYNTAX_ERROR:{
            std::cout << "Syntax Error: ";
            break;
        }
        case error::SEMANTIC_ERROR:{
            std::cout << "Semantic Error: ";
            break;
        }
        case error::CODE_GENERATION_ERROR:{
            std::cout << "Code Generation Error: ";
            break;
        }
        default:break;
    }
    std::cerr << '\'' << msg << '\'' << std::endl;
    if (line_number!=-1)
        std::cerr << "Near line " << line_number << std::endl;
    
    std::exit(1);
}

void emit_error(error err, std::string msg){
    emit_error(err,msg,-1);

}

std::string sformat(std::string str, ...){
    std::string ret;

    va_list args;
    va_start(args, str);

    for (int i=0;i<str.size();i++){
        if (str[i]=='%'){
            i++;
            switch (str[i]){
                case '%':{
                    ret+='%';
                    break;
                }
                case 'd':{
                    ret+=std::to_string(va_arg(args,int));
                    break;
                }
                case 's':{
                    ret+=va_arg(args,const char*);
                    break;
                }
                default: break;
            }
        }
        else{
            ret+=str[i];
        }

    }

    va_end(args);

    return ret;

}