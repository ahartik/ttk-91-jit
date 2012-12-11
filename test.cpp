#include "asm.hpp"
#include "compiler.hpp"
#include <iostream>


short input()
{
    short c;
    std::cin>>c;
    return c;
}

void output(short c)
{
    std::cout<<c<<"\n";
}

int main(int argc, char** argv)
{
    program prog = parse_file(argv[1]);
    compiled_program cp = compile_program(prog, input, output);
    cp();
}
