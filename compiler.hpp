#pragma once
#include "asm.hpp"
#include <functional>

class compile_error : public std::runtime_error
{
    public:
    compile_error(const std::string& what) : std::runtime_error(what)
    {}
};
typedef short input_func_t(void);
typedef void output_func_t(short);
typedef input_func_t* input_func_p;
typedef output_func_t* output_func_p;

typedef std::function<void(void)> compiled_program;

compiled_program compile_program(const program& prog, input_func_p input, output_func_p output);
