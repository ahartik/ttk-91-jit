

#include <iostream>
#include <fstream>
#include <cerrno>
#include "asm.hpp"
#include <boost/regex.hpp>

using namespace std;
using namespace boost;


regex empty("[[:space:]]*(;.*)?");
regex no_label(
"[[:space:]]*([_[:alnum:]]+)[[:space:]]+([_[:alnum:]]+)[[:space:]]*(,[[:space:]]*([=_[:alnum:]]+))?[[:space:]]*(;.*)?");
regex with_label(
"[[:space:]]*([_[:alnum:]]+)[[:space:]]+([_[:alnum:]]+)[[:space:]]+([_[:alnum:]]+)[[:space:]]*(,[[:space:]]*([=_[:alnum:]]+))?[[:space:]]*(;.*)?");


static opcode parse_opcode(const std::string& code)
{
    if (code == "IN")
        return op_in;
    if (code == "OUT")
        return op_out;
    if (code == "ADD")
        return op_add;
    if (code == "SUB")
        return op_sub;
    if (code == "MUL")
        return op_mul;
    if (code == "DIV")
        return op_div;
    if (code == "JPOS")
        return op_jpos;
    if (code == "LOAD")
        return op_load;
    return op_invalid;

}
int parse_register(const string& str)
{
    if (str.size() != 2 || str[0]!='R' || '0' > str[1] || '7' < str[1])
        throw parse_error("Invalid register name '"+str+"'");
    return str[1]-'0';
}


instruction parse_instruction(const std::string& line, bool& read)
{
    read = true;
    instruction ret;
    cmatch what;
    string tokens[7];
    if (regex_match(line.c_str(), what, with_label))
    {
        for(size_t i=0;i<what.size();i++)
            tokens[i] = string(what[i].first, what[i].second);
    } else if(regex_match(line.c_str(), what, no_label)) 
    {
        for(size_t i=0;i<what.size();i++)
            tokens[i+1] = string(what[i].first, what[i].second);
    } else if(regex_match(line.c_str(), what, empty))
    {
        read = false;
        return ret;
    } else {
        throw parse_error("Cannot parse '"+line+"'");
    }

    if (isdigit(tokens[1][0]))
        throw parse_error("Invalid label '"+tokens[1]+"'");

    ret.label = tokens[1].substr(0,8);
    string opcode = tokens[2];
    ret.op = parse_opcode(opcode);
    if (ret.op == op_invalid)
        throw parse_error("Invalid opcode '"+tokens[2]+"'");
    string loper = tokens[3];
    string roper = tokens[5];
    
    ret.lreg = parse_register(loper);
    if (roper == "")
        ret.rtype = rtype_none;
    else if (roper[0]=='=')
        ret.rtype = rtype_immediate;
    else if (roper[0]=='R')
        ret.rtype = rtype_register;
    else 
        ret.rtype = rtype_label;
    if (isdigit(roper[0]))
            throw parse_error("Invalid right operand '"+roper+"'");
    switch(ret.rtype) {
        case rtype_immediate:
            {
                errno = 0;
                long rimm = strtol(roper.c_str()+1, NULL, 10);
                if (errno)
                    throw parse_error("Invalid immediate operand '"+roper+"'");
                ret.rimm = rimm;
                if (rimm != ret.rimm)
                    throw parse_error("Immediate out of range '"+roper+"'");
            }
            break;
        case rtype_label:
            ret.jump_label = roper.substr(0,8);
            break;
        case rtype_register:
            ret.rreg = parse_register(roper);
            break;
        case rtype_none:
            break;
    }

#define INVMIS throw (ret.rtype==rtype_none?parse_error("Missing operand for '"+opcode+"'"): \
                    parse_error("Invalid right operand '"+roper+"' for '"+opcode+"'"))


    // validate
    switch (ret.op) {
        case op_add:
        case op_sub:
        case op_mul:
        case op_load:
        case op_div:
            if (ret.rtype == rtype_label || ret.rtype == rtype_none)
                INVMIS;
            break;
        case op_jpos:
            if (ret.rtype != rtype_label)
                INVMIS;
            break;
        case op_in:
        case op_out:
            if (ret.rtype != rtype_none)
                throw parse_error("Unexpected right operand '"+roper+"' for '"+opcode+"'");
            break;
        case op_invalid:
            throw logic_error("Impossible to have op_invalid");
    }
    return ret;
}
program parse_file(const string& fn)
{
    std::ifstream fin(fn.c_str());
    string line;
    program ret;
    int lineno = 1;
    while (getline(fin,line))
    {
        try {
            bool ok;
            instruction ins = parse_instruction(line, ok);
            if (ok)
                ret.push_back(ins);
        } catch(parse_error e)
        {
            std::cerr<<"Error on line "<<lineno<<":\n";
            std::cerr<<e.what()<<"\n";
            return program();
        }
        lineno++;
    }
    return ret;
}
