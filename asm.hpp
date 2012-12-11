#pragma once

#include <vector>
#include <string>
#include <exception>
#include <stdexcept>

class parse_error : public std::runtime_error
{
    public:
    parse_error(const std::string& what) : std::runtime_error(what)
    {}
};

typedef enum
{
    rtype_register,
    rtype_immediate,
    rtype_label,
    rtype_none
} operand_type;

typedef enum
{
    op_in,
    op_out,
    op_add,
    op_sub,
    op_mul,
    op_div,
    op_jpos,
    op_load,
    op_invalid
} opcode;

struct instruction
{
    opcode op;
    short lreg;
    short rreg;
    short rimm;
    operand_type rtype;
    std::string label;
    std::string jump_label;
};

typedef std::vector<instruction> program;

instruction parse_instruction(const std::string& line, bool& read);
program parse_file(const std::string& filename);
