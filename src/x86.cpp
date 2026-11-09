#include "x86.h"

#include "utils.h"

instruction::instruction(std::string opcode)
    : opcode(opcode){}

instruction::instruction(std::string opcode, std::string operand1)
    : opcode(opcode), operand1(operand1){}

instruction::instruction(std::string opcode, std::string operand1, std::string operand2)
    : opcode(opcode), operand1(operand1), operand2(operand2){}

std::string instruction::to_string(){
    std::string ret=opcode;

    if (!operand1.empty())ret+=" "+operand1;
    if (!operand2.empty())ret+=", "+operand2;

    return ret;
}

label::label(std::string title, std::string data)
    : type(label_type::DATA), title(title), dsize(data_size::BYTE)
{
    for (int i=0;i<data.size();i++){
        char c=data[i];
        if (c=='\\'){
            i++;
            if (i==data.size()){
                emit_error(error::CODE_GENERATION_ERROR,"Expected special character or number after \\");
            }

            if (is_numeric(data[i])){
                std::string buf;
                int tmp=i;
                while (i<data.size() && (i-tmp)<3 && is_numeric(data[i])){
                    buf+=data[i++];
                }
                i--;
                
                this->data.push_back(std::stoi(buf));
                
            }
            else{
                char c;
                switch (data[i]){
                    case '\\':{
                        c='\\';
                        break;
                    }
                    case 'n':{
                        c='\n';
                        break;
                    }
                    case 't':{
                        c='\t';
                        break;
                    }
                    case 'r':{
                        c='\r';
                        break;
                    }
                    case '"':{
                        c='"';
                        break;
                    }
                    case '\'':{
                        c='\'';
                        break;
                    }
                    
                    //.............................................

                    default: c=data[i];

                }

                this->data.push_back(c);

            }

        }
        else{
            this->data.push_back(c);

        }

    }

}

label::label(std::string title, data_size dsize, int size)
    : dsize(dsize), title(title), size(size), type(label_type::BSS){}

std::string label::to_string(){
    std::string str;
    str+=title+": ";

    if (type==label_type::BSS){
        switch (dsize){
            case data_size::BYTE:{
                str+="resb ";
                break;
            }
            case data_size::DWORD:{
                str+="resd ";
                break;
            }
            case data_size::QWORD:{
                str+="resq ";
                break;
            }
            case data_size::WORD:{
                str+="resw ";
                break;
            }
            default:break;
        }

        str+=std::to_string(size);
    }
    else{
        str+="db ";
        for (int i=0;i<data.size();i++){
            str+=std::to_string(data[i])+",";
        }
        str+="0";

    }

    return str;

}

asm_program::asm_program(){}

std::string asm_program::push_string(std::string str){
    label l("str"+std::to_string(string_count++),str);

    data.push_back(l);

    return l.title;
}

std::string asm_program::push_array(data_size dsize, int size){
    label l("arr"+std::to_string(array_count++),dsize,size);

    bss.push_back(l);

    return l.title;
}

void asm_program::push_global(std::string name){
    label l('?'+name,data_size::QWORD,1);

    bss.push_back(l);
}

void asm_program::push_function(asm_function* function){
    functions.push_back(function);
}

asm_function::asm_function(){}

asm_function::asm_function(std::string title)
    : title('?'+title){}

void asm_function::push(instruction ins){
    instructions.push_back(ins);
}

std::string asm_function::create_local_label(){
    std::string l=".local"+std::to_string(local_label_count++);
    local_labels.push_back(l);
    return l;

}

void asm_function::push_local_label(std::string l){
    for (int i=0;i<local_labels.size();i++){
        if (local_labels[i]==l){
            push(instruction(l+": "));
            local_labels.erase(local_labels.begin()+i);
            return;
        }
    }
}

std::string asm_function::to_string(){
    std::string ret=title+":\n";

    for (int i=0;i<instructions.size();i++){
        ret+=instructions.at(i).to_string()+'\n';
    }

    return ret;
}

std::string asm_program::to_string(){
    std::string ret="default rel\n";
    ret+="section .bss\n";

    for (int i=0;i<bss.size();i++){
        ret+=bss.at(i).to_string()+'\n';
    }

    ret+="section .data\n";

    for (int i=0;i<data.size();i++){
        ret+=data.at(i).to_string()+'\n';
    }
    
    ret+="section .text\n";

    ret+="global _start\n_start:\n";

    for (int i=0;i<_start.instructions.size();i++){
        ret+=_start.instructions.at(i).to_string()+'\n';
    }

    for (int i=0;i<functions.size();i++){
        ret+=functions.at(i)->to_string()+'\n';

    }

    return ret;

}