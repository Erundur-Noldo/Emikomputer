#include "main.h"
#include "cpu.h"
#include "im.h"
#include "module.h"

void print_registers(Cpu *cpu) {
   for(int i=0x0; i<0x10; i++) {
      printf("%02x ", cpu->reg[i]);
      if(i % 4 == 3) printf("\n");
   }
}

void print_program(Im *im) {
   for(int i=0; i<0x1000; i++) {
      printf("%x", im->im4k.memory[i]);
      if(i % 0x10 == 0x0f) printf("  ");
      if(i % 0x40 == 0x3f) printf("\n");
      if(i % 0x400 == 0x3ff) printf("\n");
   }
   for(int i=0; i<0x100; i++) {
      printf("%03x ", im->im4k.flag[i]);
      if(i % 0x08 == 0x07) printf(" ");
      if(i % 0x10 == 0x0f) printf("\n");
      if(i % 0x400 == 0x3ff) printf("\n");
   }
}

int main(int argc, char *argv[]) {
   int debug = 0;
   if(argc != 2 && argc != 3) {
      printf("please provide one source code file\n");
      return 0;
   }

   if(argc == 3) { /* r ~ register, i ~ instruction, a ~ all */
      debug = argv[2][0];
   }

   Im *im = im4k_create(argv[1]);
   if(!im) {
      printf("failed to load im4k memory\n");
      return 0;
   }

   Module **module_a = module_array_create("./module_config");
   if(!module_a) {
      printf("failed to create modules\n");
      return 0;
   }


   Cpu *cpu = cpu_create(im, module_a);
   if(cpu == NULL) {
      printf("failed to create CPU\n");
      return 0;
   }

   long int cycle_count = 0;
   while(!cpu_update(cpu, debug == 'i' || debug == 'a')) cycle_count++;
   cycle_count++; /* for the stop instruction */

   printf("number of cycles: %lu\n", cycle_count);
   if(debug == 'r' || debug == 'a') print_registers(cpu);


   return 0;
}

