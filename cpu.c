#include "cpu.h"
#include "im.h"


Cpu *cpu_create(Im *im, Module **module_a) {
   Cpu *cpu = malloc(sizeof(*cpu));

   *cpu = (Cpu) {
      .reg = {0, 0, 0, 0,  
              0, 0, 0, 0,  
              0, 0, 0, 0,  
              0, 0, 3, 0},
      /* reg#e starts at 3 to have the eqq and tru flags on */
      .im = im,

   };

   for(int i=0; i<16; i++) cpu->mod[i] = module_a[i];

   im->header.cpu_cycles_busy = &cpu->num_cycles_busy;
   return cpu;
}








void cpu_wrt(Cpu *cpu, Im *im) {
   u8 data;
   int target = im_receive(im);
   data = im_receive(im) << 4;
   data |= im_receive(im);

   cpu->reg[target] = data;
}
void cpu_cpy(Cpu *cpu, Im *im) {
   u8 source = im_receive(im);
   u8 destination = im_receive(im);

   cpu->reg[destination] = cpu->reg[source];
}
void cpu_swp(Cpu *cpu, Im *im) {
   u8 reg_1 = im_receive(im);
   u8 reg_2 = im_receive(im);

   char buffer = cpu->reg[reg_1];
   cpu->reg[reg_1] = cpu->reg[reg_2];
   cpu->reg[reg_2] = buffer;
}
enum {
   ADD, SUB, INC, DEC,
   BNG, ORR, AND, XOR,
   LSH, RSH, LRT, RRT,
   ANG, LNG, LOG, PAS
} alu_modes;
void cpu_alu(Cpu *cpu, Im *im) {
   u8 mode = im_receive(im);

   u8 a = cpu->reg[0x0];
   u8 b = cpu->reg[0x1];

   u8 result;
   switch(mode) {
      case ADD: result = a + b;                   break;
      case SUB: result = a - b;                   break;
      case INC: result = a + 0x01;                break;
      case DEC: result = a - 0x01;                break;
   
      case BNG: result = ~a;                      break;
      case ORR: result = a | b;                   break;
      case AND: result = a & b;                   break;
      case XOR: result = a ^ b;                   break;

      case LSH: result = a << 1;                  break;
      case RSH: result = a >> 1;                  break;
      case LRT: result = (a << 1) | (!!(a&0x80)); break;
      case RRT: result = (a >> 1) | ((a&0x01)<<7); break;

      case ANG: result = -a;                      break;
      case LNG: result = !a;                      break;
      case LOG: /* find the highest bit which is on */
         int i=7;
         while(i >= 0 && a >> i == 0) i--;
         result = i;
         break;
      case PAS: result = a;                       break;
   }


   bool loss;
   switch(mode) {
      case ADD: 
      case SUB: loss = ((u32)a + (u32)b) & 0x100; break;
      case LSH: loss = a & 0x80;                  break;
      case RSH: loss = a & 0x01;                  break;
      case LOG: loss = a == 0x00;                 break;
      default:  loss = 0;                         break;
   }

   cpu->reg[0x0] = result;
   cpu->reg[0xe] = 
        0x01
      | 0x02 * (result == 0)
      | 0x04 * !!(result & 0x80)
      | 0x08 * !((result & 0x80) || (result == 0))
      | 0x10 * (result & 0x01)
      | 0x20 * loss
      | 0xc0 & cpu->reg[0xe]; /* we don't want to change the 
                                two most significant bits */

}
void cpu_cmp(Cpu *cpu, Im *im) {
   u8 mode = im_receive(im);

   u8 a = cpu->reg[0x0];
   u8 b = cpu->reg[0x1];

   u8 result;
   switch(mode) {
      case ADD: result = a + b;                   break;
      case SUB: result = a - b;                   break;
      case INC: result = a + 0x01;                break;
      case DEC: result = a - 0x01;                break;
   
      case BNG: result = ~a;                      break;
      case ORR: result = a | b;                   break;
      case AND: result = a & b;                   break;
      case XOR: result = a ^ b;                   break;

      // case LSH: result = a << b;                  break;
      // case RSH: result = a >> b;                  break;
      // case LRT: result = (a << b) | (a >> (8-b)); break;
      // case RRT: result = (a >> b) | (a << (8-b)); break;

      case LSH: result = a << 1;                  break;
      case RSH: result = a >> 1;                  break;
      case LRT: result = (a << 1) | (!!(a&0x80)); break;
      case RRT: result = (a >> 1) | ((a&0x01<<7)); break;

      case ANG: result = -a;                      break;
      case LNG: result = !a;                      break;
      case LOG: /* find the highest bit which is on */
         int i=7;
         while(i >= 0 && a >> i == 0) i--;
         result = i;
         break;
      case PAS: result = a;                       break;
   }


   bool loss;
   switch(mode) {
      case ADD: 
      case SUB: loss = ((u32)a + (u32)b) & 0x100; break;
      case LSH: loss = a & 0x80;                  break;
      case RSH: loss = a & 0x01;                  break;
      case LOG: loss = a == 0x00;                 break;
      default:  loss = 0;                         break;
   }

   cpu->reg[0xe] = 
        0x01
      | 0x02 * (result == 0)
      | 0x04 * !!(result & 0x80)
      | 0x08 * !((result & 0x80) || (result == 0))
      | 0x10 * (result & 0x01)
      | 0x20 * loss
      | 0xc0 & cpu->reg[0xe]; /* we don't want to change the 
                                two most significant bits */

}



void cpu_jmp(Cpu *cpu, Im *im) {
   u8 flag = im_receive(im);
   u8 inverted = !!(0x8 & flag);
   u8 selected_flag = 0x01 << (0x7 & flag);
   u8 pointer_index = im_receive(im) << 4;
   pointer_index |= im_receive(im);

   if(!inverted && (cpu->reg[0xe] & selected_flag)
   || inverted && !((cpu->reg[0xe] & selected_flag))) {
      im_jump(im, pointer_index);
   }
}
void cpu_brn(Cpu *cpu, Im *im) {
   u8 flag = im_receive(im);
   u8 selected_flag = 0x01 << (0x7 & flag);
   u8 pointer_index = im_receive(im) << 4;
   pointer_index |= im_receive(im);

   if((cpu->reg[0xe] & selected_flag) ^ (flag & 0x8)) {
      im_branch(im, pointer_index);
   }
}




void cpu_cnt(Cpu *cpu, Im *im) {
   u8 pointer_index = im_receive(im) << 4;
   pointer_index |= im_receive(im);

   cpu->reg[0xd]--;
   // printf("current value %d\n", cpu->reg[0xd]);

   if(cpu->reg[0xd] != 0x00) {
      im_jump(im, pointer_index);
   }
}



void cpu_md0(Cpu *cpu, Im *im) {
   u8 mod_index = im_receive(im);
   Module_Input_9 input = {1, cpu->reg[0x2]};
   Module_Output output = module_md0(cpu->mod[mod_index], input);
   
   assert(output.finish);
   cpu->reg[0xe] = (cpu->reg[0xe] & 0xbf) | (0x40 * output.success);
   if(output.write) cpu->reg[3] = output.data;
}

void cpu_md1(Cpu *cpu, Im *im) {
   u8 mod_index = im_receive(im);
   Module_Input_13 input = {1, cpu->reg[0], im_receive(im)};
   Module_Output output = module_md1(cpu->mod[mod_index], input);
    
   assert(output.finish);
   cpu->reg[0xe] = (cpu->reg[0xe] & 0xbf) | (0x40 * output.success);
   if(output.write) cpu->reg[3] = output.data;
}

void cpu_md2(Cpu *cpu, Im *im) {
   u8 mod_index = im_receive(im);
   Module_Input_17 input = {1, cpu->reg[0], {im_receive(im), 0}};
   input.argument[1] = im_receive(im);
   Module_Output output = module_md2(cpu->mod[mod_index], input);
    
   assert(output.finish);
   cpu->reg[0xe] = (cpu->reg[0xe] & 0xbf) | (0x40 * output.success);
   if(output.write) cpu->reg[3] = output.data;
}


void cpu_nop(Cpu *cpu) {
   return;
}

char instruction_str[][4] = {
   "stp", "wrt", "cpy", "swp",
   "alu", "cmp", "nop", "nop",
   "jmp", "brn", "ret", "cnt",
   "nop", "md0", "md1", "md2"
};
/* returns 1 when finished, 0 otherwise */
int cpu_update(Cpu *cpu, int print_instruction) {
   if(cpu->num_cycles_busy-- > 0)
      return 0;

   assert(cpu->reg[0xf] == 0x00);

   Im *im = cpu->im;
   u8 instruction = im_receive(im);

   if(!(0x0 <= instruction && instruction <= 0xf)) printf("wrong instruction: %d\n", instruction);
   assert(0x0 <= instruction && instruction <= 0xf);

   if(print_instruction) printf("%d: %s\n", im->im4k.return_stack[0], instruction_str[instruction]);
   switch(instruction) {
      case 0x0: return 1; /* stp */
      case 0x1: cpu_wrt(cpu, im); break;
      case 0x2: cpu_cpy(cpu, im); break;
      case 0x3: cpu_swp(cpu, im); break;

      case 0x4: cpu_alu(cpu, im); break;
      case 0x5: cpu_cmp(cpu, im); break;
      case 0x6: cpu_nop(cpu); break;
      case 0x7: cpu_nop(cpu); break;

      case 0x8: cpu_jmp(cpu, im); break;
      case 0x9: cpu_brn(cpu, im); break;
      case 0xa: im_return(im); break;
      case 0xb: cpu_cnt(cpu, im); break;

      case 0xc: cpu_nop(cpu); break;
      case 0xd: cpu_md0(cpu, im); break;
      case 0xe: cpu_md1(cpu, im); break;
      case 0xf: cpu_md2(cpu, im); break;

      default: assert(0); break;
   }

   cpu->reg[0xf] = 0x00;

   return 0;
}