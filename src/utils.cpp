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
    "let","while","if","else","func",
    "return","break","continue","true","false",
    "NULL", "sizeof"
};

const std::string asm_keywords[]={
    "MOV","ADD","SUB","IDIV","IMUL","JMP","JG",
    "JGE","JL","JLE","JNE","JE","CALL","RET",
    "CMP", "SHR", "SHL", "INC","DEC","SYSCALL",
    "CALL","PUSH","POP","MOVZX",
};

const std::string registers[]={
    "RAX","RBX","RCX","RDX","RDI","RSI",
    "RBP","RSP","R8","R9","R10","R11",
    "R12","R13","R14","R15","EAX","EBX",
    "ECX","EDX","EDI","ESI","EBP","ESP",
    "AX","BX","CX","DX","DI","SI","BP",
    "SP","AL","BL","CL","DL","SIL","DIL",
    "SPL","BPL",
};

const std::string binary_operators[]={
    "+","-","*","/","%","^","&","|",
    "==","!=",">>","<<",">",">=","<","<=",

};

bool is_reg8(std::string str){
    return str=="AL" || str=="BL" || str=="CL"
        || str=="DL"  || str=="SIL" || str=="DIL"
        || str=="SPL" || str=="BPL";
    
}

bool is_reg16(std::string str){
    return str=="AX" || str=="BX" || str=="CX"
        || str=="DX"  || str=="SI" || str=="DI"
        || str=="SP" || str=="BP";
}

bool is_reg32(std::string str){
    return str=="EAX" || str=="EBX" || str=="ECX"
        || str=="EDX"  || str=="ESI" || str=="EDI"
        || str=="ESP" || str=="EBP";
}

bool is_reg64(std::string str){
    return str=="RAX" || str=="RBX" || str=="RCX"
        || str=="RDX"  || str=="RDI" || str=="RSI"
        || str=="RBP" || str=="RSP" || str=="R8"
        || str=="R9" || str=="R10" || str=="R11"
        || str=="R12" || str=="R13" || str=="R14";
}

bool is_asm_keyword(std::string &str){
    for (std::string as:asm_keywords){
        if (as==str)return true;
    }
    return false;
}

bool is_binary_operator(std::string str){
    if (str.empty())return false;
    
    for (std::string op:binary_operators){
        if (op==str)return true;
    }
    
    return false;

}

bool is_register(std::string str){
    if (str=="R15")emit_error(error::SYNTAX_ERROR,"Register R15 is reserved");
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

void is_hex(std::string str){
    for (int i=2;i<str.size();i++){
        if (str[i]<'0' || (!(str[i]>='a' && str[i]<='f') && !(str[i]>='A' && str[i]<='F'))){
            emit_error(error::LEXICAL_ERROR, "Invalid hexadecimal literal '"+str+"'");
            
        }

    }

}

void is_binary(std::string str){
    for (int i=2;i<str.size();i++){
        if (str[i]<'0' || str[i]>'1'){
            emit_error(error::LEXICAL_ERROR, "Invalid binary literal '"+str+"'");
        }
    }

}

bool is_numeric(std::string &str){
    if (str.size()>2 && str[0]=='0' && str[1]=='x'){
        is_hex(str);
        return true;

    }
    else if (str.size()>2 && str[0]=='0' && str[1]=='b'){
        is_binary(str);
        return true;

    }
    for (char c:str){
        if (!is_numeric(c)){
            if (is_numeric(str[0])){
                emit_error(error::LEXICAL_ERROR,"Invalid number literal '"+str+"'");
            }
            return false;
        }
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

int get_asm_instruction_length(std::string &keyword){
    if (keyword=="RET" || keyword=="SYSCALL")return 1;
    if (keyword=="CALL" || keyword=="INC" || keyword=="DEC" 
        || keyword=="IDIV" || keyword=="JMP" || keyword=="JG" 
        || keyword=="JGE" || keyword=="JL" || keyword=="JLE" 
        || keyword=="JE" || keyword=="JNE" || keyword=="PUSH" 
        || keyword=="POP")return 2;
    
    return 3;

}