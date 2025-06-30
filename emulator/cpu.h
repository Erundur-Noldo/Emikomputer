#ifndef CPU_H
#define CPU_H



#include "main.h"
#include "im.h"
#include "module.h"






typedef struct Cpu Cpu;
struct Cpu {
   u8 reg[16];

   Im *im;
   Module *mod[16];

   s32 num_cycles_busy;
};

Cpu *cpu_create(Im *im, Module **module_a);
int cpu_update(Cpu *cpu, int print_instruction);




#endif /* CPU_H */
