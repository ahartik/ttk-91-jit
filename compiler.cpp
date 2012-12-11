#include "compiler.hpp"
#include "jit/jit-plus.h"
#include <cassert>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <array>

using namespace std;

typedef array<short, 8> register_file;
jit_type_t input_signature = jit_function::signature_helper(jit_type_short, jit_function::end_params);
jit_type_t output_signature = jit_function::signature_helper(jit_type_void, jit_type_short, jit_function::end_params);

class program_function : public jit_function
{
    program prog;
    input_func_p input;
    output_func_p output;
    jit_type_t short_pointer_type;
    program_function(const program_function& other);
    unordered_map<string, jit_label> label_map;
    public:
    program_function(jit_context& context, const program& p,
            input_func_p in,
            output_func_p out) :
        jit_function(context),
        prog(p),
        input(in),
        output(out)
    {
        short_pointer_type = jit_type_create_pointer(jit_type_short, 0);
        create();
    }
    virtual void build()
    {
        jit_value reg = insn_load(get_param(0));
        for(size_t pc = 0; pc < prog.size(); pc++)
        {
            const instruction& ins = prog[pc];
            if (ins.label != "")
            {
                insn_label(label_map[ins.label]);
            }
            switch(ins.op) {
                case op_out:
                    {
                        jit_value x = insn_load_relative(reg, ins.lreg * 2, jit_type_short);
                        jit_value_t xr = x.raw();
                        insn_call_native("output", (void*)output, output_signature, &xr, 1, 0);
                    }
                    break;
                case op_in:
                    {
                        jit_value x = insn_call_native("input", (void*)input,
                                input_signature, NULL, 0, 0);
                        insn_store_relative(reg, ins.lreg * 2, x);
                    }
                    break;
                case op_add:
                case op_mul:
                case op_sub:
                case op_div:
                case op_load:
                    {
                        jit_value x = insn_load_relative(reg, ins.lreg * 2, jit_type_short);
                        jit_value y;
                        jit_value z;
                        if (ins.rtype == rtype_register)
                            y = insn_load_relative(reg, ins.rreg * 2, jit_type_short);
                        else
                            y = new_constant(ins.rimm);
                        switch(ins.op) {
                            case op_add:
                                z = x + y;
                                break;
                            case op_mul:
                                z = x * y;
                                break;
                            case op_sub:
                                z = x - y;
                                break;
                            case op_div:
                                z = x / y;
                                break;
                            case op_load:
                                z = y;
                                break;
                            default:
                                assert(!"IMPOSSIBLE");
                        };
                        z = insn_convert(z, jit_type_short, 0);
                        insn_store_relative(reg, ins.lreg * 2, z);
                        break;
                    }
                case op_jpos:
                    {
                        jit_value x = insn_load_relative(reg, ins.lreg * 2, jit_type_short);
                        jit_value cmp = x > new_constant((short)0);
                        insn_branch_if(cmp, label_map[ins.jump_label]);
                        break;
                    }
                case op_invalid:
                    cerr<<"Unsupported opcode\n";
                    return;
            };
        }
        insn_return();
    }
    protected:
    virtual jit_type_t create_signature()
    {
        return signature_helper(jit_type_void,
                short_pointer_type, end_params);
    }
};

struct compiled_program_functor {
    shared_ptr<program_function> pf; 
    void operator()()
    {

        register_file regs{1,2,3,4,5,6,7,8};
        jit_short* arg = &regs[0];
        void* args[1];
        args[0] = &arg;
        pf->apply(args, NULL);
    };
};

static jit_context jcont;
compiled_program compile_program(const program& prog,input_func_p in,output_func_p out)
{
    shared_ptr<program_function> func(new program_function(jcont, prog, in, out));
    
    compiled_program_functor ret {func};
    return ret;
}
