#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <fstream>
#include <cstdint>
#include <filesystem>

#include "utils.h"
#include "token.h"
#include "ast.h"
#include "symbol.h"
#include "x86.h"

std::string input_file,output_file;

void print_help(){
    printf("C- Compiler\n\nUsage: c- [INPUT_FILE] [OUTPUT_FILE] [OPTION]...\n\nOptions:\n\t--help\n\t    Print help information.\n\t-a\n\t    Output asm file only\n\n");

}

std::string get_file_as_string(std::string file_name){
    std::ifstream s(file_name);
    std::string ret,line;
    
    while (getline(s,line)){
        ret=ret+line+"\n";

    }

    return ret;

}

std::string get_dir(std::string filename){
    int i;
    for (i=filename.size()-1;i>=0 && filename[i]!='/';i--);
    if (filename[i]!='/')return "";
    return filename.substr(0,i+1);

}

std::string get_file_as_string(std::string dir, std::string filename){
    if (dir.empty())return get_file_as_string(filename);
    return get_file_as_string(dir+"/"+filename);
}

token_list lex(std::string &file){
    token_list list;
    std::string buf;

    int line_number=1;

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
                case ',':{
                    token t2(token_type::DELIMITER);
                    t2.value=",";
                    t2.line_number=line_number;
                    list.push(t2);
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
                        if (file[i+1]=='-' || file[i+1]=='=' || file[i+1]=='>'){
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

ast_node* parse_statement(token_list &list);
ast_node* parse_expression(token_list &list);
ast_node* parse_logical_or(token_list &list);
ast_node* parse_shift(token_list &list);

ast_node* parse_primary(token_list &list){

    if (list.match(token_type::NUMBER)){
        ast_node* node=new ast_node(ast_node_type::NUMBER);
        
        node->value=list++.value; //advance past number literal

        return node;
    }
    else if (list.match(token_type::IDENTIFIER)){
        ast_node* node=new ast_node(ast_node_type::IDENTIFIER);

        node->identifier=list++.value;
        
        if (list.match("(")){
            int tmp_line_number=list.peek().line_number;
            list++; //advance past '('
            node->type=ast_node_type::FUNCTION_CALL;
            
            while (!list.end() && !list.match(")")){
                ast_node* param=parse_expression(list);
                node->args.push(param);

                if (list.match(")"))break;
                list.expect(",");

            }

            list.expect(")");

        }

        return node;

    }
    else if (list.match("sizeof")){
        ast_node* node=new ast_node(ast_node_type::SIZEOF);
        list++;

        list.expect("(");
        token t=list.expect(token_type::IDENTIFIER);
        list.expect(")");

        node->identifier=t.value;
        return node;
    }
    else if (list.match(token_type::REGISTER)){
        ast_node* node=new ast_node(ast_node_type::REGISTER);
        node->value=list++.value;

        return node;
    }
    else if (list.match("(")){
        list++;

        ast_node* node=parse_expression(list);

        list.expect(")");

        return node;
    }
    else if (list.match("true") || list.match("false")){
        ast_node* node=new ast_node(ast_node_type::NUMBER);
        node->value=list.match("true")?"1":"0";
        list++;
        return node;
    }
    else if (list.match("NULL")){
        ast_node* node=new ast_node(ast_node_type::NUMBER);
        node->value="0";
        list++;
        return node;
    }
    else if (list.match(token_type::STRING)){
        ast_node* node=new ast_node(ast_node_type::STRING);
        node->value=list.expect(token_type::STRING).value;
        return node;
    }

    return nullptr;

}

ast_node* parse_member_access(token_list &list){
    ast_node* node=parse_primary(list);

    if (list.match(".") || list.match("->")){
        ast_node* node2=new ast_node(ast_node_type::MEMBER_ACCESS);
        node2->value=list++.value;

        node2->left=node;
        node2->right=parse_member_access(list);

        node=node2;
    }

    return node;
}

ast_node* parse_array_access(token_list &list){
    ast_node* node=parse_member_access(list);

    if (list.match("$")){
        ast_node* node2=new ast_node(ast_node_type::ARRAY_ACCESS);

        list.expect("$");
        node2->element_size_reference=parse_expression(list);

        do{
            list.expect("[");
            ast_node* index=parse_expression(list);
            node2->indexes.push(index);
            list.expect("]");

        }while(list.match("["));

        node2->left=node;
        node=node2;

    }

    return node;

}

ast_node* parse_postfix(token_list &list){
    ast_node* node=parse_array_access(list);

    if (list.match("++") || list.match("--")){
        ast_node* node2=new ast_node(ast_node_type::EXPRESSION);
        node2->value=list++.value;

        node2->left=node;
        node=node2;
    }

    return node;
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

    return parse_postfix(list);

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
    ast_node* node=parse_equals(list);
    return node;
}

ast_node* parse_array(token_list &list){
    ast_node* node=new ast_node(ast_node_type::ARRAY);
    list.expect("{");

    do{
        ast_node* element;

        if (list.match("{")){
            element=parse_array(list);
        }
        else{
            element=parse_expression(list);
        }
        node->elements.push(element);

        if (list.match("}")){
            break;
        }
        list.expect(",");
        

    }while(!list.match("}"));

    list.expect("}");

    return node;

}

ast_node* parse_declaration(token_list &list){
    list++;

    ast_node* node=new ast_node(ast_node_type::DECLARATION);

    node->identifier=list.expect(token_type::IDENTIFIER).value;

    if (list.match("$")){
        list++;

        int lineno=list.peek().line_number;

        node->type=ast_node_type::ARRAY_DECLARATION;
        node->element_size_reference=parse_expression(list);

        if (node->element_size_reference->type!=ast_node_type::NUMBER){
            emit_error(error::SYNTAX_ERROR,"Element size reference must be constant in declaration",lineno);
        }

        do{
            list.expect("[");

            int lineno2=list.peek().line_number;

            ast_node* index=parse_expression(list);
            if (index->type!=ast_node_type::NUMBER){
                emit_error(error::SYNTAX_ERROR,"Array size(s) must be constant in declaration", lineno2);
            }
            node->indexes.push(index);
            list.expect("]");

        }while (list.match("["));

        if (list.match(";")){
            list.expect(";");
            return node;
        }
        
        list.expect("=");

        node->right=parse_array(list);

        list.expect(";");

        return node;

    }

    if (list.match(";")){
        list.expect(";");
        return node;
    }

    list.expect("=");

    node->right=parse_expression(list);

    list.expect(";");

    return node;

}

ast_node* parse_block(token_list &list){
    ast_node* node=new ast_node(ast_node_type::BLOCK);
    list++;
    
    while (!list.end() && !list.match("}")){
        ast_node* statement=parse_statement(list);
        if (statement!=nullptr)node->children.push(statement);
    }

    list.expect("}");

    return node;

}

ast_node* parse_function(token_list &list){
    ast_node* node=new ast_node(ast_node_type::FUNCTION_DEFINITION);
    list++;

    node->function_declaration=false;

    node->identifier=list.expect(token_type::IDENTIFIER).value;

    list.expect("(");

    while (!list.end() && !list.match(")")){
        token param=list.expect(token_type::IDENTIFIER);

        node->params.push_back(param);

        if (list.match(")"))break;

        list.expect(",");

    }

    list.expect(")");

    list.expect("{");

    while (!list.end() && !list.match("}")){
        ast_node* statement=parse_statement(list);

        if (statement!=nullptr)node->children.push(statement);

    }

    list.expect("}");

    return node;
    
}

ast_node* parse_return(token_list &list){
    ast_node* node=new ast_node(ast_node_type::RETURN);

    list.expect("return");

    if (!list.match(";")){
        node->right=parse_expression(list);
    }

    list.expect(";");

    return node;

}

ast_node* parse_break(token_list &list){
    ast_node* node=new ast_node(ast_node_type::BREAK);
    list.expect("break");
    list.expect(";");

    return node;
}

ast_node* parse_continue(token_list &list){
    ast_node* node=new ast_node(ast_node_type::CONTINUE);
    list.expect("continue");
    list.expect(";");

    return node;
}

ast_node* parse_while(token_list &list){
    ast_node* node=new ast_node(ast_node_type::LOOP);
    list.expect("while");

    list.expect("(");

    node->condition=parse_expression(list);

    list.expect(")");

    node->right=parse_statement(list);

    return node;

}

ast_node* parse_if(token_list &list){
    ast_node* node=new ast_node(ast_node_type::IF);
    list.expect("if");

    list.expect("(");

    node->condition=parse_expression(list);

    list.expect(")");

    node->right=parse_statement(list);

    if (list.match("else")){
        ast_node* else_node=new ast_node(ast_node_type::ELSE);
        list.expect("else");

        else_node->right=parse_statement(list);

        node->else_node=else_node;

    }

    return node;
        
}

void parse_asm_mem_operand(token_list &list, ast_node* node, bool operand1){
    list++;
    std::string &operand=operand1?node->operand1:node->operand2;
    operand='[';
    do{
        token t=list++;
        if (t.type==token_type::OPERATOR){
            if (operand.size()==1 || (t.value!="+" && t.value!="*" && t.value!="-")){
                emit_error(error::SYNTAX_ERROR, "Operator '"+t.value+"' not allowed here",t.line_number);
            }
            operand+=t.value;
            t=list++;
        }
        if (t.type!=token_type::REGISTER && t.type!=token_type::NUMBER){
            emit_error(error::SYNTAX_ERROR,"'"+token_type_to_string(t.type)+"' not allowed here",t.line_number);
        }

        operand+=t.value;

    }while (!list.end() && !list.match("]"));

    list.expect("]");

    operand+=']';

}

ast_node* parse_asm_statement(token_list &list){
    ast_node* node=new ast_node(ast_node_type::ASM);

    std::string keyword=list++.value;
    int instruction_length=get_asm_instruction_length(keyword);

    node->opcode=keyword;

    if (instruction_length==1){
        list.expect(";");
        return node;
    }

    bool is_mem=false;

    if (list.match("[")){
        is_mem=true;
        parse_asm_mem_operand(list,node,true);

    }
    else if (is_register(list.peek().value)){
        node->operand1=list++.value;

    }
    else{
        node->operand1_node=parse_expression(list);

    }

    if (instruction_length==2){
        list.expect(";");
        return node;
    }

    list.expect(",");

    if (list.match("[")){
        if (is_mem || node->operand1.empty()){
            emit_error(error::SYNTAX_ERROR,"Cannot have two memory operands",list.peek().line_number);
        }
        parse_asm_mem_operand(list,node,false);
    }
    else if (is_register(list.peek().value)){
        node->operand2=list++.value;
    }
    else{
        node->operand2_node=parse_expression(list);

    }

    list.expect(";");

    return node;

}

ast_node* parse_struct(token_list &list){
    list++; //advance past let
    list++; //advance past struct

    ast_node* node=new ast_node(ast_node_type::STRUCT);
    node->identifier=list.expect(token_type::IDENTIFIER).value;

    list.expect("{");

    while (!list.match("}")){
        ast_node* decl=parse_statement(list);

        if (decl!=nullptr && decl->type!=ast_node_type::ARRAY_DECLARATION && decl->type!=ast_node_type::DECLARATION){
            emit_error(error::SYNTAX_ERROR,"Illegal statement within struct '"+node->identifier+"' definition");
        }
        if (decl->right){
            emit_error(error::SYNTAX_ERROR,"Assignments not allowed in struct definition");
        }
        if (decl){
            node->children.push(decl);
        }
    }
    list.expect("}");
    
    return node;

}

ast_node* parse_statement(token_list &list){

    if (list.match(token_type::KEYWORD)){
        if (list.match("let")){
            if (list.lookahead().value=="struct"){
                return parse_struct(list);
            }
            return parse_declaration(list);
        }
        if (list.match("func")){
            return parse_function(list);
        }
        if (list.match("return")){
            return parse_return(list);
        }
        if (list.match("while")){
            return parse_while(list);
        }
        if (list.match("break")){
            return parse_break(list);
        }
        if (list.match("continue")){
            return parse_continue(list);
        }
        if (list.match("if")){
            return parse_if(list);
        }

    }
    else if (list.match(token_type::ASM)){
        return parse_asm_statement(list);
    }
    else if (list.match("{")){
        return parse_block(list);
    }
    else{
        ast_node* node=parse_expression(list);
        list.expect(";");
        return node;
    }

}

ast parse(token_list &list){
    ast tree;

    while (!list.end()){
        ast_node* node=parse_statement(list);
        if (node!=nullptr)tree.push(node);
    }
    
    return tree;
    
}

void build_symbol_table(ast_node* node, symbol_table* table){   
    if (node==nullptr)return;

    if (node->type==ast_node_type::DECLARATION || node->type==ast_node_type::ARRAY_DECLARATION){
        symbol* s=new symbol(node->identifier);

        if (table->type!=symbol_table_type::GLOBAL){
            s->type=symbol_type::LOCAL;
            s->offset=table->cur_offset;
            
            table->cur_offset-=8;
        }
        else{
            s->type=symbol_type::GLOBAL;
        }
        
        table->push(s);
        
    }
    else if (node->type==ast_node_type::FUNCTION_DEFINITION){
        symbol* s=new symbol(node->identifier);
        s->type=symbol_type::FUNCTION;
        table->push(s);
    
        node->table=new symbol_table(symbol_table_type::FUNCTION,table,-8);
        for (int i=0;i<node->children.size();i++)
            build_symbol_table(node->children.get(i),node->table);
        
        node->table->push_params(node->params);

    }
    else if (node->type==ast_node_type::BLOCK){
        if (table->type==symbol_table_type::GLOBAL){
            emit_error(error::SEMANTIC_ERROR,"Unexpected block statement");

        }
        node->table=new symbol_table(symbol_table_type::BLOCK,table,table->cur_offset);
        for (int i=0;i<node->children.size();i++)
            build_symbol_table(node->children.get(i),node->table);

    }
    else if (node->type==ast_node_type::LOOP){
        build_symbol_table(node->right,table);

    }
    else if (node->type==ast_node_type::IF){
        build_symbol_table(node->right,table);
        if (node->else_node)
            build_symbol_table(node->else_node->right,table);
        
    }
    

}

void generate_expression(asm_program &program, asm_function* function, symbol_table* table, ast_node* node, bool get_address){
    if (node==nullptr)return;
    
    if (is_binary_operator(node->value) && node->type==ast_node_type::EXPRESSION){
        generate_expression(program,function,table,node->left, false);
        function->push(instruction("PUSH","RAX"));
        function->push(instruction("PUSH","R8"));
        generate_expression(program,function,table,node->right, false);
        function->push(instruction("POP","R8"));
        function->push(instruction("MOV","RBX","RAX"));
        function->push(instruction("POP","RAX"));

        
        if (node->value=="+"){
            function->push(instruction("ADD","RAX","RBX"));

        }
        else if (node->value=="-"){
            function->push(instruction("SUB","RAX","RBX"));

        }
        else if (node->value=="*"){
            function->push(instruction("IMUL","RAX","RBX"));
        }
        else if (node->value=="/" || node->value=="%"){
            function->push(instruction("CQO"));
            function->push(instruction("IDIV","RBX"));
            
            if (node->value=="%"){
                function->push(instruction("MOV","RAX","RDX"));
            }

        }
        else if (node->value==">>"){
            function->push(instruction("SHR","RAX","RBX"));
        }
        else if (node->value=="<<"){
            function->push(instruction("SHL","RAX","RBX"));
        }
        else if (node->value=="=="){
            function->push(instruction("CMP","RAX","RBX"));
            function->push(instruction("SETE","AL"));
            function->push(instruction("MOVZX","RAX","AL"));
        }
        else if (node->value=="!="){
            function->push(instruction("CMP","RAX","RBX"));
            function->push(instruction("SETNE","AL"));
            function->push(instruction("MOVZX","RAX","AL"));
        }
        else if (node->value=="<"){
            function->push(instruction("CMP","RAX","RBX"));
            function->push(instruction("SETL","AL"));
            function->push(instruction("MOVZX","RAX","AL"));
        }
        else if (node->value=="<="){
            function->push(instruction("CMP","RAX","RBX"));
            function->push(instruction("SETLE","AL"));
            function->push(instruction("MOVZX","RAX","AL"));
        }
        else if (node->value==">"){
            function->push(instruction("CMP","RAX","RBX"));
            function->push(instruction("SETG","AL"));
            function->push(instruction("MOVZX","RAX","AL"));
        }
        else if (node->value==">="){
            function->push(instruction("CMP","RAX","RBX"));
            function->push(instruction("SETGE","AL"));
            function->push(instruction("MOVZX","RAX","AL"));
        }
        else if (node->value=="^"){
            function->push(instruction("XOR","RAX","RBX"));
        }
        else if (node->value=="&"){
            function->push(instruction("AND","RAX","RBX"));
        }
        else if (node->value=="|"){
            function->push(instruction("OR","RAX","RBX"));
        }


    }
    else if (node->type==ast_node_type::REGISTER){
        if (node->value!="RAX"){
            if (!is_reg64(node->value)){
                function->push(instruction("MOVZX","RAX",node->value));
            }
            else{
                function->push(instruction("MOV","RAX","node->value"));
            }
            
        }
        
    }
    else if (node->type==ast_node_type::UNARY){

        if (node->value=="*"){
            generate_expression(program,function,table,node->right,false);

            if (!get_address){
                function->push(instruction("MOV","RAX","[RAX]"));
            }

        }
        else if (node->value=="+"){
            generate_expression(program,function,table,node->right,false);
        }
        else if (node->value=="!"){
            generate_expression(program,function,table,node->right,false);
            function->push(instruction("CMP","RAX","0"));
            function->push(instruction("SETE","AL"));
            function->push(instruction("MOVZX","RAX","AL"));
        }
        else if (node->value=="~"){
            generate_expression(program,function,table,node->right,false);
            function->push(instruction("NOT","RAX"));
        }
        else if (node->value=="-"){
            generate_expression(program,function,table,node->right,false);
            function->push(instruction("NEG","RAX"));
        }
        else if (node->value=="@"){
            generate_expression(program,function,table,node->right,true);

        }
        else if(node->value=="++" || node->value=="--"){
            std::string opc=node->value=="++"?"ADD":"SUB";

            generate_expression(program,function,table,node->right,true);

            std::string _2=function->create_local_label();
            std::string _4=function->create_local_label();
            std::string _8=function->create_local_label();
            std::string donel=function->create_local_label();

            function->push(instruction("MOV","RBX","1"));

            function->push(instruction("CMP","R8","2"));
            function->push(instruction("JGE",_2));
            function->push(instruction(opc,"[RAX]","BL"));
            function->push(instruction("JMP",donel));
            function->push_local_label(_2);
            function->push(instruction("CMP","R8","4"));
            function->push(instruction("JGE",_4));
            function->push(instruction(opc,"[RAX]","BX"));
            function->push(instruction("JMP",donel));
            function->push_local_label(_4);
            function->push(instruction("CMP","R8","8"));
            function->push(instruction("JGE",_8));
            function->push(instruction(opc,"[RAX]","EBX"));
            function->push(instruction("JMP",donel));
            function->push_local_label(_8);
            function->push(instruction(opc,"[RAX]","RBX"));
            function->push_local_label(donel);
            function->push(instruction("MOV","R8","8"));

            generate_expression(program,function,table,node->right,false);

        }

    }
    else if (node->value=="&&"){
        std::string donel=function->create_local_label();

        function->push(instruction("MOV","RBX","0"));
        generate_expression(program,function,table,node->left,false);
        function->push(instruction("CMP","RAX","0"));
        function->push(instruction("JE",donel));
        generate_expression(program,function,table,node->right,false);
        function->push(instruction("CMP","RAX","0"));
        function->push(instruction("JE",donel));
        function->push(instruction("MOV","RBX","1"));
        function->push_local_label(donel);
        function->push(instruction("MOV","RAX","RBX"));

    }
    else if (node->value=="||"){
        std::string donel=function->create_local_label();

        function->push(instruction("MOV","RBX","1"));
        generate_expression(program,function,table,node->left,false);
        function->push(instruction("CMP","RAX","0"));
        function->push(instruction("JNE",donel));
        generate_expression(program,function,table,node->right,false);
        function->push(instruction("CMP","RAX","0"));
        function->push(instruction("JNE",donel));
        function->push(instruction("MOV","RBX","0"));
        function->push_local_label(donel);
        function->push(instruction("MOV","RAX","RBX"));

    }
    else if (node->value=="++" || node->value=="--"){
        std::string opc=node->value=="++"?"ADD":"SUB";

        generate_expression(program,function,table,node->left,false);

        function->push(instruction("PUSH","RAX"));

        generate_expression(program,function,table,node->left,true);

        std::string _2=function->create_local_label();
        std::string _4=function->create_local_label();
        std::string _8=function->create_local_label();
        std::string donel=function->create_local_label();

        function->push(instruction("MOV","RBX","1"));

        function->push(instruction("CMP","R8","2"));
        function->push(instruction("JGE",_2));
        function->push(instruction(opc,"[RAX]","BL"));
        function->push(instruction("JMP",donel));
        function->push_local_label(_2);
        function->push(instruction("CMP","R8","4"));
        function->push(instruction("JGE",_4));
        function->push(instruction(opc,"[RAX]","BX"));
        function->push(instruction("JMP",donel));
        function->push_local_label(_4);
        function->push(instruction("CMP","R8","8"));
        function->push(instruction("JGE",_8));
        function->push(instruction(opc,"[RAX]","EBX"));
        function->push(instruction("JMP",donel));
        function->push_local_label(_8);
        function->push(instruction(opc,"[RAX]","RBX"));
        function->push_local_label(donel);
        function->push(instruction("MOV","R8","8"));

        function->push(instruction("POP","RAX"));

    }
    else if (node->type==ast_node_type::ARRAY_ACCESS){
        generate_expression(program,function,table,node->left,false);
        function->push(instruction("PUSH","RAX"));
        function->push(instruction("PUSH","R8"));

        generate_expression(program,function,table,node->element_size_reference,false);

        function->push(instruction("POP","R8"));
        function->push(instruction("MOV","R8","RAX"));

        function->push(instruction("POP","RAX")); //RAX contains address

        for (int i=0;i<node->indexes.size();i++){
            function->push(instruction("PUSH","RAX"));
            function->push(instruction("PUSH","R8"));

            generate_expression(program,function,table,node->indexes.get(i),false);

            function->push(instruction("POP","R8"));

            function->push(instruction("MOV","RBX","RAX"));
            function->push(instruction("POP","RAX"));

            if (i<node->indexes.size()-1){
                function->push(instruction("MOV","RAX","[RAX+RBX*8]"));

            }
            else{
                function->push(instruction("IMUL","RBX","R8"));
                function->push(instruction("LEA","RAX","[RAX+RBX]"));

            }

        }

        if (!get_address){
            std::string _2=function->create_local_label();
            std::string _4=function->create_local_label();
            std::string _8=function->create_local_label();
            std::string donel=function->create_local_label();

            function->push(instruction("CMP","R8","2"));
            function->push(instruction("JGE",_2));
            function->push(instruction("MOV","AL","[RAX]"));
            function->push(instruction("MOVZX","RAX","AL"));
            function->push(instruction("JMP",donel));
            function->push_local_label(_2);
            function->push(instruction("CMP","R8","4"));
            function->push(instruction("JGE",_4));
            function->push(instruction("MOV","AX","[RAX]"));
            function->push(instruction("MOVZX","RAX","AX"));
            function->push(instruction("JMP",donel));
            function->push_local_label(_4);
            function->push(instruction("CMP","R8","8"));
            function->push(instruction("JGE",_8));
            function->push(instruction("MOV","EAX","[RAX]"));
            function->push(instruction("MOVZX","RAX","EAX"));
            function->push(instruction("JMP",donel));
            function->push_local_label(_8);
            function->push(instruction("MOV","RAX","[RAX]"));
            function->push_local_label(donel);
            function->push(instruction("MOV","R8","8"));

        }
            
    }
    else if (node->value=="=" || node->value=="+=" || node->value=="-=" || 
        node->value=="^=" || node->value=="&=" || node->value=="|="){
            std::string opc;
            if (node->value=="=")opc="MOV";
            else if (node->value=="+=")opc="ADD";
            else if (node->value=="-=")opc="SUB";
            else if (node->value=="^=")opc="XOR";
            else if (node->value=="|=")opc="OR";
            else if (node->value=="&=")opc="AND";

            generate_expression(program,function,table,node->left,true);

            function->push(instruction("PUSH","RAX"));
            function->push(instruction("PUSH","R8"));

            generate_expression(program,function,table,node->right,false);

            function->push(instruction("POP","R8"));
            function->push(instruction("MOV","RBX","RAX"));
            function->push(instruction("POP","RAX"));

            std::string _2=function->create_local_label();
            std::string _4=function->create_local_label();
            std::string _8=function->create_local_label();
            std::string donel=function->create_local_label();

            function->push(instruction("CMP","R8","2"));
            function->push(instruction("JGE",_2));
            function->push(instruction(opc,"[RAX]","BL"));
            function->push(instruction("JMP",donel));
            function->push_local_label(_2);
            function->push(instruction("CMP","R8","4"));
            function->push(instruction("JGE",_4));
            function->push(instruction(opc,"[RAX]","BX"));
            function->push(instruction("JMP",donel));
            function->push_local_label(_4);
            function->push(instruction("CMP","R8","8"));
            function->push(instruction("JGE",_8));
            function->push(instruction(opc,"[RAX]","EBX"));
            function->push(instruction("JMP",donel));
            function->push_local_label(_8);
            function->push(instruction(opc,"[RAX]","RBX"));
            function->push_local_label(donel);
            function->push(instruction("MOV","R8","8"));

            function->push(instruction("MOV","RAX","RBX"));

        }
    else if (node->value=="/=" || node->value=="%="){
        generate_expression(program,function,table,node->left,false);

        function->push(instruction("PUSH","RAX"));
        function->push(instruction("PUSH","R8"));

        generate_expression(program,function,table,node->right,false);

        function->push(instruction("POP","R8"));
        function->push(instruction("MOV","RBX","RAX"));

        function->push(instruction("POP","RAX"));

        function->push(instruction("CQO"));

        function->push(instruction("IDIV","RBX"));

        function->push(instruction("PUSH","RBX"));

        if (node->value=="%="){
            function->push(instruction("MOV","RAX","RDX"));
        }

        function->push(instruction("MOV","RBX","RAX"));

        generate_expression(program,function,table,node->left,true);

        std::string _2=function->create_local_label();
        std::string _4=function->create_local_label();
        std::string _8=function->create_local_label();
        std::string donel=function->create_local_label();

        function->push(instruction("CMP","R8","2"));
        function->push(instruction("JGE",_2));
        function->push(instruction("MOV","[RAX]","BL"));
        function->push(instruction("JMP",donel));
        function->push_local_label(_2);
        function->push(instruction("CMP","R8","4"));
        function->push(instruction("JGE",_4));
        function->push(instruction("MOV","[RAX]","BX"));
        function->push(instruction("JMP",donel));
        function->push_local_label(_4);
        function->push(instruction("CMP","R8","8"));
        function->push(instruction("JGE",_8));
        function->push(instruction("MOV","[RAX]","EBX"));
        function->push(instruction("JMP",donel));
        function->push_local_label(_8);
        function->push(instruction("MOV","[RAX]","RBX"));
        function->push_local_label(donel);
        function->push(instruction("MOV","R8","8"));

        function->push(instruction("POP","RAX"));

    }
    else if (node->value==">>=" || node->value=="<<=" || node->value=="*="){
        std::string opc=node->value=="*="?"IMUL"
            :node->value==">>="?"SHR"
            :"SHL";

        generate_expression(program,function,table,node->left,false);

        function->push(instruction("PUSH","RAX"));

        generate_expression(program,function,table,node->right,false);

        function->push(instruction("MOV","RBX","RAX"));
        function->push(instruction("POP","RAX"));

        function->push(instruction(opc,"RAX","RBX"));

        function->push(instruction("PUSH","RBX"));
        function->push(instruction("MOV","RBX","RAX"));

        generate_expression(program,function,table,node->left,true);

        std::string _2=function->create_local_label();
        std::string _4=function->create_local_label();
        std::string _8=function->create_local_label();
        std::string donel=function->create_local_label();

        function->push(instruction("CMP","R8","2"));
        function->push(instruction("JGE",_2));
        function->push(instruction("MOV","[RAX]","BL"));
        function->push(instruction("JMP",donel));
        function->push_local_label(_2);
        function->push(instruction("CMP","R8","4"));
        function->push(instruction("JGE",_4));
        function->push(instruction("MOV","[RAX]","BX"));
        function->push(instruction("JMP",donel));
        function->push_local_label(_4);
        function->push(instruction("CMP","R8","8"));
        function->push(instruction("JGE",_8));
        function->push(instruction("MOV","[RAX]","EBX"));
        function->push(instruction("JMP",donel));
        function->push_local_label(_8);
        function->push(instruction("MOV","[RAX]","RBX"));
        function->push_local_label(donel);
        function->push(instruction("MOV","R8","8"));

        function->push(instruction("POP","RAX"));

    }
    else if (node->type==ast_node_type::FUNCTION_CALL){
        symbol* s=table->lookup(node->identifier);
        if (s==nullptr){
            emit_error(error::CODE_GENERATION_ERROR,"Unknown function '"+node->identifier+"'");
        }
        for (int i=node->args.size()-1;i>=0;i--){
            ast_node* arg=node->args.get(i);

            generate_expression(program, function, table, arg, false);

            function->push(instruction("PUSH","RAX"));
        }

        function->push(instruction("LEA","RAX",sformat("[%s]",s->get_address().c_str())));

        function->push(instruction("CALL","RAX"));

        if (node->args.size()){
            function->push(instruction("ADD","RSP",std::to_string(node->args.size()*8)));
        }

    }
    else if (node->value=="?"){
        std::string elif=function->create_local_label();
        std::string donel=function->create_local_label();

        generate_expression(program,function,table,node->condition,false);

        function->push(instruction("CMP","RAX","0"));
        function->push(instruction("JE",elif));
        generate_expression(program,function,table,node->true_node,false);
        function->push(instruction("JMP",donel));
        function->push_local_label(elif);
        generate_expression(program,function,table,node->false_node,false);
        function->push_local_label(donel);

    }
    else if (node->type==ast_node_type::NUMBER){
        function->push(instruction("MOV","RAX",node->value));
    }
    else if (node->type==ast_node_type::STRING){
        std::string str=program.push_string(node->value);
        function->push(instruction("MOV","RAX",str));
    }
    else if (node->type==ast_node_type::IDENTIFIER){
        symbol* s=table->lookup(node->identifier);
        if (s==nullptr){
            emit_error(error::CODE_GENERATION_ERROR,sformat("Unkown symbol '%s'",node->identifier.c_str()));
        }

        std::string address=s->get_address();

        function->push(instruction("LEA","RAX",sformat("[%s]",address.c_str())));

        if (!get_address){
            function->push(instruction("MOV","RAX","[RAX]"));
        }

    }

}

void generate_array(asm_program &program, asm_function* function, ast &indexes, int index, int esr){
    int cur_size=std::stoi(indexes.get(index)->value);

    bool last=index==indexes.size()-1;

    data_size dsize;
    if (last){
        if (esr<2)dsize=data_size::BYTE;
        else if (esr<4)dsize=data_size::WORD;
        else if (esr<8)dsize=data_size::DWORD;
        else if (esr==8)dsize=data_size::QWORD;
        else {
            cur_size=1+((cur_size*esr)/8);
            dsize=data_size::QWORD;
        }
    }
    else{
        dsize=data_size::QWORD;
    }

    std::string array=program.push_array(dsize,cur_size);

    for (int i=0;i<cur_size && !last;i++){
        generate_array(program,function,indexes,index+1,esr);
        function->push(instruction("MOV",sformat("[%s+%d*8]",array.c_str(),i),"RAX"));

    }

    function->push(instruction("MOV","RAX",array));

}

void generate_fill_array(asm_program &program, asm_function* function, symbol_table* table, ast &elements, int esr){
    std::string reg;

    if (esr<2)reg="BL";
    else if (esr<4)reg="BX";
    else if (esr<8)reg="EBX";
    else reg="RBX";

    for (int i=0;i<elements.size();i++){
        ast_node* node=elements.get(i);

        if (node->type==ast_node_type::ARRAY){
            function->push(instruction("PUSH","RAX"));
            function->push(instruction("MOV","RAX",sformat("[RAX+%d*8]",i)));

            generate_fill_array(program,function,table,node->elements,esr);

            function->push(instruction("POP","RAX"));

        }
        else{
            function->push(instruction("PUSH","RAX"));

            generate_expression(program,function,table,node,false);

            function->push(instruction("MOV","RBX","RAX"));

            function->push(instruction("POP","RAX"));

            function->push(instruction("MOV",sformat("[RAX+%d*%d]",i,esr),reg));

        }

    }

}

void generate_declaration(asm_program &program, asm_function* function, symbol_table* table, ast_node* node){
    symbol* s=table->lookup(node->identifier);
    
    if (s->type==symbol_type::GLOBAL){
        program.push_global(s->name);
    }

    if (node->type==ast_node_type::ARRAY_DECLARATION){
        int esr=std::stoi(node->element_size_reference->value);

        generate_array(program,function,node->indexes,0,esr);

        if (node->right){
            function->push(instruction("PUSH","RAX"));

            generate_fill_array(program,function,table,node->right->elements,esr);
            
            function->push(instruction("POP","RAX"));

        }


    }
    else{
        generate_expression(program,function,table,node->right,false);

    }
    function->push(instruction("MOV",sformat("[%s]",s->get_address().c_str()),"RAX"));

}

void generate_statement(asm_program &program, asm_function* function, symbol_table* table, ast_node* node);

void generate_block(asm_program &program, asm_function* function, ast_node* node){
    
    for (int i=0;i<node->children.size();i++){
        ast_node* statement=node->children.get(i);
        generate_statement(program,function,node->table,statement);
    }

}

void generate_epilogue(asm_function* function){
    function->push(instruction("MOV","RSP","RBP"));
    function->push(instruction("POP","RBP"));
    function->push(instruction("RET"));

}

void generate_return(asm_program &program, asm_function* function, symbol_table* table, ast_node* node){
    generate_expression(program,function,table,node->right,false);
    generate_epilogue(function);

}

std::vector<std::string> continue_labels;
std::vector<std::string> break_labels;

void generate_loop(asm_program &program, asm_function* function, symbol_table* table, ast_node* node){
    std::string start=function->create_local_label();
    std::string end=function->create_local_label();

    continue_labels.push_back(start);
    break_labels.push_back(end);

    function->push_local_label(start);

    generate_expression(program,function,table,node->condition,false);
    
    function->push(instruction("CMP","RAX","0"));
    function->push(instruction("JE",end));

    generate_statement(program,function,table,node->right);

    function->push(instruction("JMP",start));

    function->push_local_label(end);
    
    continue_labels.pop_back();
    break_labels.pop_back();

}

void generate_break(asm_function* function){
    if (break_labels.empty())emit_error(error::CODE_GENERATION_ERROR,"Break outside loop");

    function->push(instruction("JMP",break_labels.at(break_labels.size()-1)));
    
}

void generate_continue(asm_function* function){
    if (continue_labels.empty())emit_error(error::CODE_GENERATION_ERROR,"Continue outside loop");

    function->push(instruction("JMP",continue_labels.at(continue_labels.size()-1)));

}

void generate_if(asm_program &program, asm_function* function, symbol_table* table, ast_node* node){
    std::string endl=function->create_local_label();
    std::string notif=function->create_local_label();
    
    generate_expression(program,function,table,node->condition,false);

    function->push(instruction("CMP","RAX","0"));
    function->push(instruction("JE",notif));
    
    generate_statement(program,function,table,node->right);
    
    function->push(instruction("JMP",endl));

    function->push_local_label(notif);

    if (node->else_node)generate_statement(program,function,table,node->else_node->right);

    function->push_local_label(endl);

}

void pushaq(asm_function* function){
    function->push(instruction("PUSH","RAX"));
    function->push(instruction("PUSH","RBX"));
    function->push(instruction("PUSH","RCX"));
    function->push(instruction("PUSH","RDX"));
    function->push(instruction("PUSH","RDI"));
    function->push(instruction("PUSH","RSI"));
    function->push(instruction("PUSH","RBP"));
    function->push(instruction("PUSH","RSP"));
    function->push(instruction("PUSH","R8"));
    function->push(instruction("PUSH","R9"));
    function->push(instruction("PUSH","R10"));
    function->push(instruction("PUSH","R11"));
    function->push(instruction("PUSH","R12"));
    function->push(instruction("PUSH","R13"));
    function->push(instruction("PUSH","R14"));

}

void popaq(asm_function* function){
    function->push(instruction("POP","R14"));
    function->push(instruction("POP","R13"));
    function->push(instruction("POP","R12"));
    function->push(instruction("POP","R11"));
    function->push(instruction("POP","R10"));
    function->push(instruction("POP","R9"));
    function->push(instruction("POP","R8"));
    function->push(instruction("POP","RSP"));
    function->push(instruction("POP","RBP"));
    function->push(instruction("POP","RSI"));
    function->push(instruction("POP","RDI"));
    function->push(instruction("POP","RDX"));
    function->push(instruction("POP","RCX"));
    function->push(instruction("POP","RBX"));
    function->push(instruction("POP","RAX"));

}

void generate_asm(asm_program &program, asm_function* function, symbol_table* table, ast_node* node){
    std::string opcode=node->opcode;

    int instruction_length=get_asm_instruction_length(opcode);

    if (instruction_length==1){
        function->push(instruction(opcode));
        return;
    }

    if (instruction_length==2){
        if (node->operand1_node==nullptr){
            function->push(instruction(opcode,node->operand1));
        }
        else{
            generate_expression(program,function,table,node->operand1_node,false);
            function->push(instruction(opcode,"RAX"));
        }
        return;
    }

    std::string op1,op2;

    if (node->operand1_node==nullptr){
        op1=node->operand1;

    }
    else{
        pushaq(function);
        generate_expression(program,function,table,node->operand1_node,true);
        function->push(instruction("MOV","R15","RAX"));
        popaq(function);
        op1="[R15]";

    }
    if (node->operand2_node==nullptr){
        op2=node->operand2;
    }
    else{
        pushaq(function);
        generate_expression(program,function,table,node->operand2_node,false);
        function->push(instruction("MOV","R15","RAX"));
        popaq(function);
        op2="R15";

    }

    function->push(instruction(opcode,op1,op2));

}

void generate_statement(asm_program &program, asm_function* function, symbol_table* table, ast_node* node){
    if (node==nullptr)return;

    if (node->type==ast_node_type::DECLARATION || node->type==ast_node_type::ARRAY_DECLARATION){
        generate_declaration(program,function,table,node);
    }
    else if (node->type==ast_node_type::BLOCK){
        generate_block(program,function,node);
    }
    else if (node->type==ast_node_type::RETURN){
        generate_return(program,function,table,node);
    }
    else if (node->type==ast_node_type::LOOP){
        generate_loop(program,function,table,node);
    }
    else if (node->type==ast_node_type::CONTINUE){
        generate_continue(function);
    }
    else if (node->type==ast_node_type::BREAK){
        generate_break(function);
    }
    else if (node->type==ast_node_type::IF){
        generate_if(program,function,table,node);
    }
    else if (node->type==ast_node_type::ASM){
        generate_asm(program,function,table,node);
    }
    else{
        generate_expression(program,function,table,node,false);
    }

}

void generate_function(asm_program &program, ast_node* node){
    asm_function* function=new asm_function(node->identifier);

    symbol_table* table=node->table;

    function->push(instruction("PUSH","RBP"));
    function->push(instruction("MOV","RBP","RSP"));
    if (table->locals_count)
        function->push(instruction("SUB","RSP",std::to_string(table->locals_count*8)));

    for (int i=0;i<node->children.size();i++){
        ast_node* statement=node->children.get(i);

        generate_statement(program,function,table,statement);

    }

    generate_epilogue(function);

    program.push_function(function);

}

void generate_code(asm_program &program, symbol_table* table, ast &tree){
    program._start.push(instruction("MOV","R8","8"));

    {
        std::string tmp="main";

        if (table->lookup(tmp)==nullptr){
            emit_error(error::CODE_GENERATION_ERROR,"No main function");
        }
    }
    
    for (int i=0;i<tree.size();i++){
        ast_node* node=tree.get(i);

        if (node->type==ast_node_type::ARRAY_DECLARATION || node->type==ast_node_type::DECLARATION){
            generate_declaration(program,&program._start,table,node);

        }
        else if (node->type==ast_node_type::FUNCTION_DEFINITION){
            if (!node->function_declaration){
                generate_function(program, node);
            }
        }

    }

    program._start.push(instruction("MOV","RAX","[RSP]"));
    program._start.push(instruction("LEA","RBX","[RSP+8]"));

    program._start.push(instruction("PUSH","RBX"));
    program._start.push(instruction("PUSH","RAX"));
    program._start.push(instruction("CALL","?main"));

    program._start.push(instruction("ADD","RSP","16"));
    program._start.push(instruction("MOV","RDI","RAX"));
    program._start.push(instruction("MOV","RAX","60"));
    program._start.push(instruction("SYSCALL"));

}

std::vector<std::filesystem::path> paths;

bool paths_contains(std::filesystem::path name){
    for (int i=0;i<paths.size();i++){
        if (paths.at(i)==name)return true;
    }
    return false;
}

std::string expand(std::filesystem::path path){
    std::string src=get_file_as_string(path.string());

    token_list list=lex(src);

    std::filesystem::path dir=path.parent_path();
    
    std::string out;

    while (list.seti("#include")){
        list++;
        std::string name=list.peek().value;
        std::filesystem::path inc;

        if (name.rfind("~/",0)==0){
            name=name.substr(2);
            name=std::getenv("HOME")+("/"+name);
            inc=name;
        }else{
            inc=dir/name;
        }
        if (!paths_contains(inc)){
            paths.push_back(inc);
            out+=expand(inc);
        }

    }

    return out+src;

}

void replace(token_list &list){
    typedef struct replace_t{
        std::string str;
        token replacement;
        replace_t(std::string str, token replacement){
            this->str=str;
            this->replacement=replacement;
        }

    }replace_t;

    std::vector<replace_t> replacements;

    while (list.seti("#define")){
        int i=list.i;
        list++;

        std::string str=list++.value;
        token replacement=list++;

        replacements.push_back(replace_t(str,replacement));

        for (int j=0;j<3;j++){
            list.remove(i);
        }
        
    }

    for (replace_t r:replacements){
        for (int i=0;i<list.size();i++){
            if (list.at(i).value==r.str){
                list.set(i,r.replacement);
            }
        }
    }

    list.seti(0);
}

void get_includes(std::string &filestr){
    filestr=expand(std::filesystem::path(input_file));

}

void remove_includes(token_list &list){
    while (list.seti("#include")){
        int i=list.i;
        list.remove(i);
        list.remove(i);
    }
    list.seti(0);
}

int main(int argc, char** argv){
    std::cout << "MUST HAVE NASM INSTALLED!" << std::endl << std::endl;

    bool asm_only=false;

    char* whoami=argv[0];
    
    for (int i=1;i<argc;i++){
        if ((std::string)argv[i]=="--help"){
            print_help();
            return 0;
        }
        if ((std::string)argv[i]=="-a"){
            asm_only=true;
            continue;

        }

    }

    if (argc<3){
        std::cerr << "Incorrect number of args!" << std::endl;
        return 1;
        
    }

    input_file=argv[1],output_file=argv[2];

    std::string file;

    get_includes(file);

    token_list tokens=lex(file);

    remove_includes(tokens);

    replace(tokens);
    
    ast tree=parse(tokens);
    tree.print(0);
    return 0;
    symbol_table* global_table=new symbol_table(symbol_table_type::GLOBAL);

    for (int i=0;i<tree.size();i++)
        build_symbol_table(tree.get(i),global_table);

    asm_program program;

    generate_code(program,global_table,tree);

    std::string nasm_string=program.to_string();

    std::string nasmstr=output_file+".asm";

    std::ofstream nasm_file(nasmstr);

    nasm_file << nasm_string;

    nasm_file.close();

    std::string objstr=output_file+".o";

    std::string cmd1="nasm -f elf64 "+nasmstr+" -o "+objstr;
    std::string cmd2="ld "+objstr+" -o "+output_file;
    std::string cmd3="rm "+nasmstr+" "+objstr;

    if (!asm_only){
        std::system(cmd1.c_str());
        std::system(cmd2.c_str());
        std::system(cmd3.c_str());

    }
    
    return 0;
}