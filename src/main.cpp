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

    int line_number=0;

    for (uint32_t i=0;i<file.length();++i){
        char c=file[i];

        if (!is_delimiter(c)){
            buf+=c;
        }
        else{
            if (!buf.empty()){
                token t(buf);
                t.line_number=line_number;

                list.push(t);
                buf.clear();
            }

            switch (c){
                case ' ':{
                    continue;
                }
                case '\n':{
                    line_number++;
                    continue;
                }
                case '\t':{
                    continue;
                }
                case '"':{
                    i++;
                    
                    while (i<file.length() && file[i]!='"'){
                        buf+=file[i++];
                    }

                    if (i==file.length()){
                        emit_error(error::LEXICAL_ERROR,"Expected closing \"",line_number);
                    }

                    token t2(token_type::STRING,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();

                    continue;
                }
                case '\'':{
                    i++;
                    if (i==file.length()){
                        emit_error(error::LEXICAL_ERROR,"Expected a character after '",line_number);
                    }

                    if (file[i]=='\\'){
                        i++;
                        
                        if (i==file.length())
                            emit_error(error::LEXICAL_ERROR,"Expected character after \\",line_number);
                        
                        char special=file[i];
                        
                        if (is_numeric(special)){
                            while (i<file.length() && file[i]!='\''){
                                buf+=file[i++];
                            }
                            if (i==file.length()){
                                emit_error(error::LEXICAL_ERROR,"Expected closing '",line_number);
                            }

                            token t2(token_type::NUMBER,buf);
                            t2.line_number=line_number;
                            list.push(t2);

                            buf.clear();
                            

                        }
                        else{
                            int num;
                            switch (special){
                                case 'n':{
                                    num='\n';
                                    break;
                                }
                                case 't':{
                                    num='\t';
                                    break;
                                }
                                case '\'':{
                                    num='\'';
                                    break;
                                }
                                case '"':{
                                    num='"';
                                    break;
                                }
                                case '\\':{
                                    num='\\';
                                    break;
                                }
                                //....................................................................
                            }
                            buf=std::to_string(num);

                            i++;
                            if (i==file.length() || file[i]!='\''){
                                emit_error(error::LEXICAL_ERROR,"Expected closing '",line_number);
                            }

                            token t2(token_type::NUMBER,buf);
                            t2.line_number=line_number;
                            list.push(t2);
                            buf.clear();
                            
                            
                        }

                    }
                    else{
                        buf+=file[i++];

                        if (i==file.length() || file[i]!='\''){
                            emit_error(error::LEXICAL_ERROR,"Expected closing '",line_number);
                        }
                        token t2(token_type::NUMBER,std::to_string((int)buf[0]));
                        t2.line_number=line_number;
                        list.push(t2);
                    
                        buf.clear();
                        
                    }

                    continue;

                }
                case '+':{
                    buf='+';
                    if (i!=file.length()-1){
                        if (file[i+1]=='+' || file[i+1]=='='){
                            buf+=file[i+1];
                            i++;

                        }

                    }
                    token t2(token_type::OPERATOR,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case '-':{
                    buf='-';
                    if (i!=file.length()-1){
                        if (file[i+1]=='-' || file[i+1]=='='){
                            buf+=file[i+1];
                            i++;
                        }
                    }
                    token t2(token_type::OPERATOR,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case '*':{
                    buf='*';
                    if (i!=file.length()-1){
                        if (file[i+1]=='='){
                            buf+=file[i+1];
                            i++;
                        }
                    }
                    token t2(token_type::OPERATOR,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case '/':{
                    buf='/';
                    if (i!=file.length()-1){
                        if (file[i+1]=='='){
                            buf+=file[i+1];
                            i++;
                        }
                        else if (file[i+1]=='/'){
                            while (i<file.length() && file[i]!='\n'){
                                i++;
                            }
                            buf.clear();
                            if (file[i]=='\n')line_number++;
                            continue;
                        }
                        else if (file[i+1]=='*'){
                            i+=2;
                            int tmp_line_number=line_number;
                            while (i<file.length()-1 && (file[i]!='*' || file[i+1]!='/')){
                                if (file[i]=='\n')line_number++;
                                i++;
                            }
                            if (i==file.length()-1){
                                emit_error(error::LEXICAL_ERROR, "Expected closing */",tmp_line_number);
                            }
                            i++;
                            buf.clear();
                            continue;
                        }
                    }
                    token t2(token_type::OPERATOR,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case '>':{
                    buf='>';
                    if (i!=file.length()-1){
                        if (file[i+1]=='=' || file[i+1]=='>'){
                            buf+=file[i+1];
                            i++;
                        }
                        if (file[i]=='>' && i<file.length()-1 && file[i+1]=='='){
                            buf+='=';
                            i++;
                        }
                    }
                    token t2(token_type::OPERATOR,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case '<':{
                    buf='<';
                    if (i!=file.length()-1){
                        if (file[i+1]=='<' || file[i+1]=='='){
                            buf+=file[i+1];
                            i++;
                        }
                        if (file[i]=='<' && i<file.length()-1 && file[i+1]=='='){
                            buf+='=';
                            i++;
                        }
                    }
                    token t2(token_type::OPERATOR,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case '=':{
                    buf='=';
                    if (i!=file.length()-1){
                        if (file[i+1]=='='){
                            buf+=file[i+1];
                            i++;
                        }
                    }
                    token t2(token_type::OPERATOR,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case '@':{
                    buf='@';

                    token t2(token_type::OPERATOR,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case '~':{
                    buf='~';

                    token t2(token_type::OPERATOR,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case '&':{
                    buf='&';
                    if (i!=file.length()-1){
                        if (file[i+1]=='&' || file[i+1]=='='){
                            buf+=file[i+1];
                            i++;
                        }
                    }
                    token t2(token_type::OPERATOR,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case '|':{
                    buf='|';
                    if (i!=file.length()-1){
                        if (file[i+1]=='|' || file[i+1]=='='){
                            buf+=file[i+1];
                            i++;
                        }
                    }
                    token t2(token_type::OPERATOR,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case '^':{
                    buf='^';
                    if (i!=file.length()-1){
                        if (file[i+1]=='='){
                            buf+=file[i+1];
                            i++;
                        }
                    }
                    token t2(token_type::OPERATOR,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case ':':{
                    buf=':';
                    
                    token t2(token_type::DELIMITER,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case ';':{
                    buf=';';
                    token t2(token_type::DELIMITER,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case '%':{
                    buf='%';
                    if (i!=file.length()-1){
                        if (file[i+1]=='='){
                            buf+=file[i+1];
                            i++;
                        }
                    }
                    token t2(token_type::OPERATOR,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case '!':{
                    buf='!';
                    if (i!=file.length()-1){
                        if (file[i+1]=='='){
                            buf+=file[i+1];
                            i++;
                        }
                    }
                    token t2(token_type::OPERATOR,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case '?':{
                    buf='?';
                    token t2(token_type::OPERATOR,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case '(':{
                    buf='(';
                    token t2(token_type::DELIMITER,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case ')':{
                    buf=')';
                    
                    token t2(token_type::DELIMITER,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case '[':{
                    buf='[';
                    token t2(token_type::DELIMITER,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case ']':{
                    buf=']';
                    
                    token t2(token_type::DELIMITER,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case '{':{
                    buf='{';
                    
                    token t2(token_type::DELIMITER,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case '}':{
                    buf='}';
                    
                    token t2(token_type::DELIMITER,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case '$':{
                    buf='$';
                    
                    token t2(token_type::OPERATOR,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                case '.':{
                    buf='.';
                    
                    token t2(token_type::OPERATOR,buf);
                    t2.line_number=line_number;
                    list.push(t2);

                    buf.clear();
                    continue;

                }
                default:break;

            }

        }

    }
    if (!buf.empty()){
        token t(buf);

        list.push(t);

    }
    return list;

}

ast_node* parse_primary(token_list &list){

    if (list.match(token_type::NUMBER)){
        ast_node* node=new ast_node(ast_node_type::NUMBER);
        
        node->value=list++.value; //advance past number literal

        return node;
    }
    else if (list.match(token_type::IDENTIFIER)){
        ast_node* node=new ast_node(ast_node_type::IDENTIFIER);

        node->identifier=list++.value; //advance past identifier

        if (list.match("++") || list.match("--")){
            node->type=ast_node_type::EXPRESSION;
            node->value=list++.value; //advance past postfix
            
        }
        else if (list.match("(")){
            list++; //advance past '('
            int tmp_line_number=list.peek().line_number;
            node->type=ast_node_type::FUNCTION_CALL;
            
            while (!list.end() && !list.match(")")){
                ast_node* param=parse_expression(list);
                node->params.push(param);

                if (list.match(")"))break;
                list.expect(",");

            }

            // list.peek(); //check for EOF

            list.expect(")");

        }

        return node;

    }
    else if (list.match("(")){
        ast_node* node=new ast_node(ast_node_type::EXPRESSION);
        list++; //advance past '('

        node->right=parse_expression(list);

        list.expect(")");

        return node;
    }
    else{
        emit_error(error::SYNTAX_ERROR,("Unexpected token '"+list.peek().value+"'").c_str(),list.peek().line_number);
    }

}

ast_node* parse_unary(token_list &list){

    if (list.match("+") || list.match("-") || list.match("!") || 
        list.match("~") || list.match("@") || list.match("++") || 
        list.match("--") || list.match("*")){
        ast_node* node=new ast_node(ast_node_type::UNARY);

        node->value=list++.value;
        
        node->right=parse_unary(list);

        return node;

    }

    return parse_primary(list);

}

ast_node* parse_multiplicative(token_list &list){
    ast_node* node=parse_unary(list);

    while (list.match("*") || list.match("/") || list.match("%")){
        ast_node* new_node=new ast_node(ast_node_type::EXPRESSION);
        new_node->value=list++.value;

        new_node->left=node;
        new_node->right=parse_unary(list);
        
        node=new_node;
    }

    return node;
}

ast_node* parse_additive(token_list &list){
    ast_node* node=parse_multiplicative(list);

    while (list.match("+") || list.match("-")){
        ast_node* new_node=new ast_node(ast_node_type::EXPRESSION);
        new_node->value=list++.value;

        new_node->left=node;
        new_node->right=parse_multiplicative(list);
        
        node=new_node;
    }

    return node;
}

ast_node* parse_shift(token_list &list){
    ast_node* node=parse_additive(list);

    while (list.match(">>") || list.match("<<")){
        ast_node* new_node=new ast_node(ast_node_type::EXPRESSION);
        new_node->value=list++.value;

        new_node->left=node;
        new_node->right=parse_additive(list);
        
        node=new_node;
    }

    return node;
}

ast_node* parse_comparison(token_list &list){
    ast_node* node=parse_shift(list);

    while (list.match(">") || list.match(">=") || list.match("<") || list.match("<=")){
        ast_node* new_node=new ast_node(ast_node_type::EXPRESSION);
        new_node->value=list++.value;

        new_node->left=node;
        new_node->right=parse_shift(list);
        
        node=new_node;
    }
    
    return node;
}

ast_node* parse_equality(token_list &list){
    ast_node* node=parse_comparison(list);

    while (list.match("==") || list.match("!=")){
        ast_node* new_node=new ast_node(ast_node_type::EXPRESSION);
        new_node->value=list++.value;

        new_node->left=node;
        new_node->right=parse_comparison(list);
        
        node=new_node;
    }

    return node;
}

ast_node* parse_bitwise_and(token_list &list){
    ast_node* node=parse_equality(list);

    while (list.match("&")){
        ast_node* new_node=new ast_node(ast_node_type::EXPRESSION);
        new_node->value=list++.value;

        new_node->left=node;
        new_node->right=parse_equality(list);
        
        node=new_node;
    }

    return node;
}

ast_node* parse_xor(token_list &list){
    ast_node* node=parse_bitwise_and(list);

    while (list.match("^")){
        ast_node* new_node=new ast_node(ast_node_type::EXPRESSION);
        new_node->value=list++.value;

        new_node->left=node;
        new_node->right=parse_bitwise_and(list);
        
        node=new_node;
    }

    return node;
}

ast_node* parse_bitwise_or(token_list &list){
    ast_node* node=parse_xor(list);

    while (list.match("|")){
        ast_node* new_node=new ast_node(ast_node_type::EXPRESSION);
        new_node->value=list++.value;

        new_node->left=node;
        new_node->right=parse_xor(list);
        
        node=new_node;
    }
    
    return node;
}

ast_node* parse_logical_and(token_list &list){
    ast_node* node=parse_bitwise_or(list);

    while (list.match("&&")){
        ast_node* new_node=new ast_node(ast_node_type::EXPRESSION);
        new_node->value=list++.value;

        new_node->left=node;
        new_node->right=parse_logical_or(list);
        
        node=new_node;
    }

    return node;
}

ast_node* parse_logical_or(token_list &list){
    ast_node* node=parse_logical_and(list);

    while (list.match("||")){
        ast_node* new_node=new ast_node(ast_node_type::EXPRESSION);
        new_node->value=list++.value;

        new_node->left=node;
        new_node->right=parse_logical_and(list);
        
        node=new_node;
    }

    return node;
}

ast_node* parse_ternary(token_list &list){
    ast_node* node=parse_logical_or(list);

    if (list.match("?")){
        ast_node* new_node=new ast_node(ast_node_type::EXPRESSION);
        new_node->value=list++.value;

        new_node->condition=node;
        new_node->true_node=parse_expression(list);
        list.expect(":");
        new_node->false_node=parse_expression(list);
        node=new_node;
    }

    return node;
}

ast_node* parse_equals(token_list &list){
    ast_node *node=parse_ternary(list);

    if (list.match("=") || list.match("+=") || list.match("-=") || 
        list.match("*=") || list.match("/=") || list.match("^=") || 
        list.match("&=") || list.match("|=") || list.match("%=") || 
        list.match(">>=") || list.match("<<=")){
        ast_node* new_node=new ast_node(ast_node_type::EXPRESSION);

        new_node->value=list++.value;

        new_node->left=node;
        new_node->right=parse_equals(list);

        node=new_node;

    }
    return node;
}

ast_node* parse_expression(token_list &list){
    return parse_equals(list);
}

ast_node* parse_declaration(token_list &list){
    ast_node* node=new ast_node(ast_node_type::DECLARATION);

    list++; //advance past '='
    token identifier_token=list++; //advance past identifier

    if (identifier_token.type!=token_type::IDENTIFIER){
        emit_error(error::SYNTAX_ERROR,"Expected identifier after let",identifier_token.line_number);
    }

    node->value=identifier_token.value;
    
    list.expect("=");

    node->right=parse_expression(list);
    
    list.expect(";");

    return node;

}

ast_node* parse_statement(token_list &list){

    if (list.match(token_type::KEYWORD)){
        if (list.match("let")){
            return parse_declaration(list);
        }

    }
    else{

    }

}

ast parse(token_list &list){
    ast tree;

    while (!list.end()){
        ast_node* node=parse_statement(list);
        tree.push(node);

    }
    
    return tree;
    
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

    token_list tokens=lex(file);

    std::cout << tokens.to_string() << std::endl;

    return 0;
}