#pragma once

#include <string>
#include <vector>

// enum class opcode{
//     MOV,
//     ADD,
//     SUB,
//     IMUL,
//     IDIV,
//     NEG,
//     NOT,
//     AND,
//     OR,
//     XOR,
//     JE,
//     JNE,
//     JGE,
//     JG,
//     JL,
//     JLE,
//     JMP,
//     CALL,
//     RET,
//     SYSCALL,
//     CMP,
//     MOVZX,
//     MOVSX,
//     PUSH,
//     POP,
//     INC,
//     DEC,

// };

enum class data_size{
    BYTE=1,
    WORD=2,
    DWORD=4,
    QWORD=8,

};

enum class label_type{
    BSS,
    DATA,
        
};

class instruction{
    public:
    std::string opcode,operand1,operand2;

    instruction(std::string opcode);

    instruction(std::string opcode, std::string operand1);

    instruction(std::string opcode, std::string operand1, std::string operand2);

    std::string to_string();

    private:
};

class label{
    public:
    label_type type;
    data_size dsize;
    std::string title;
    std::vector<char> data;
    int size;

    label(std::string title, std::string data);

    label(std::string title, data_size dsize, int size);

    std::string to_string();

    private:
};

class asm_function{
    public:
    std::string title;
    std::vector<instruction> instructions;
    
    std::vector<std::string> local_labels;

    int local_label_count=0;

    asm_function();

    asm_function(std::string title);

    void push(instruction ins);

    std::string create_local_label();

    void push_local_label(std::string l);

    private:
};

class asm_program{
    public:
    std::vector<label> data,bss;

    int string_count=0,array_count=0;

    std::vector<asm_function*> functions;
    asm_function _start;
    
    asm_program();

    void push_function(asm_function* function);

    std::string push_string(std::string str);

    std::string push_array(data_size dsize, int size);

    void push_global(std::string name);

    std::string to_string();

    private:
};