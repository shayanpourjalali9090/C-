#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <fstream>
#include <cstdint>

#include "utils.h"
#include "token.h"
#include "ast.h"
#include "symbol.h"
#include "x86_instruction.h"


std::string input_file,output_file;

void print_help(){
    printf("C- Compiler\n\nUsage: c- [OPTION]... [INPUT_FILE] [OUTPUT_FILE]\n\nOptions:\n\t--help\n\t    Print help information.\n\n");

}

std::string get_file_as_string(std::string &file_name){
    std::ifstream s(file_name);
    std::string ret,line;
    
    while (getline(s,line)){
        ret=ret+line+"\n";

    }

    return ret;

}

token_list lex(std::string &file){
    token_list list;
    std::string buf;

    for (uint32_t i=0;i<file.length();i++){
        char c=file[i];

        if (!is_delimiter(c)){
            buf+=c;
        }
        else{
            if (!buf.empty()){
                token t;
                t.type=is_numeric(buf)?token_type::NUMBER:is_keyword(buf)?token_type::KEYWORD:token_type::IDENTIFIER;
                t.value=buf;
            }

            

        }

    }
    if (!buf.empty()){

    }
    return list;

}

int main(int argc, char** argv){
    std::cout << "MUST HAVE NASM INSTALLED!" << std::endl << std::endl;

    for (int i=1;i<argc;i++){
        if ((std::string)argv[i]=="--help"){
            print_help();
            return 0;
        }

    }

    if (argc<3){
        std::cerr << "Incorrect number of args!" << std::endl;
        return 1;
        
    }

    input_file=argv[1],output_file=argv[2];

    std::string file=get_file_as_string(input_file);
    
    return 0;
}