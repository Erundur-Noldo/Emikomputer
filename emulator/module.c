#include "main.h"
#include "module.h"
#include <string.h>


Module *no_module_create() {
   Module *module = malloc(sizeof(module->no_module));
   if(!module) return NULL;

   module->type = NO_MODULE;
   return module;
}
Module *out1_create() {
   Module *module = malloc(sizeof(module->out1));
   if(!module) return NULL;

   module->out1 = (Out1) {OUT1};
   return module;
}
Module_Output out1_md0(Module *module, Module_Input_9 input) {
   printf("%02x   (%03d, ", input.data, input.data);
   for(int i=7; i>=0; i--) {
      printf("%d", !!(input.data & (1 << i))); 
   }
   printf(")\n");
   return (Module_Output) {true, true, false, 0};
}
Module_Output out1_md1(Module *module, Module_Input_13 input) {
   return (Module_Output) {true, false, false, 0};
}
Module_Output out1_md2(Module *module, Module_Input_17 input) {
   return (Module_Output) {true, false, false, 0};
}



Module *out256_create() {
   Module *module = malloc(sizeof(module->out256));
   if(!module) return NULL;

   module->out256 = (Out256) {OUT256, {0}};
   return module;
}
Module_Output out256_md0(Module *module, Module_Input_9 input) {
   module->out256.pixel[input.data] = !module->out256.pixel[input.data];
   for(int i=0; i<256; i++) {
      printf("%c", module->out256.pixel[i] ? 'o' : '.');
      if(i % 16 == 15) printf("\n");
   }
   printf("\n");
   return (Module_Output) {true, true, false, 0};
}
Module_Output out256_md1(Module *module, Module_Input_13 input) {
   module->out256.pixel[input.data] = !!input.argument;
   for(int i=0; i<256; i++) {
      printf("%c", module->out256.pixel[i] ? 'o' : '.');
      if(i % 16 == 15) printf("\n");
   }
   printf("\n");
   return (Module_Output) {true, true, false, 0};
}
Module_Output out256_md2(Module *module, Module_Input_17 input) {
   return (Module_Output) {true, false, false, 0};
}






Module *in1_create() {
   Module *module = malloc(sizeof(module->out1));
   if(!module) return NULL;

   module->in1 = (In1) {IN1};
   return module;
}
Module_Output in1_md0(Module *module, Module_Input_9 input) {
   printf("input 2-digit hex number: ");
   int user_input = 5;
   scanf("%02x", &user_input);
   return (Module_Output) {
      .finish = true,
      .success = true,
      .write = true,
      .data = (char) user_input
   };
}
Module_Output in1_md1(Module *module, Module_Input_13 input) {
   return (Module_Output) {true, false, false, 0};
}
Module_Output in1_md2(Module *module, Module_Input_17 input) {
   return (Module_Output) {true, false, false, 0};
}




Module *stack256_create() {
   Module *module = malloc(sizeof(module->stack256));
   if(!module) return NULL;

   module->stack256 = (Stack256) {
      STACK256,
      0,
      0,
      {0}
   };
   return module;
}
Module_Output stack256_md0(Module *module, Module_Input_9 input) {
   assert(module->stack256.mode == 0 || module->stack256.mode == 1 || module->stack256.mode == 2);

   return stack256_md1(module, (Module_Input_13) {1, input.data, 0x04 | module->stack256.mode});

}
Module_Output stack256_md1(Module *module, Module_Input_13 input) {
   if(!(input.argument & 0x04)) {
      module->stack256.mode = input.argument;
      return (Module_Output) {true, true, false, 0};
   }

   char output;
   switch(module->stack256.mode & 0x03) {
      case 0: /* push */
         if(module->stack256.top == 0x100)
            return (Module_Output) {true, false, false, 0};

         module->stack256.data[module->stack256.top++] = input.data;
         return (Module_Output) {true, true, false, 0};

      case 1: /* pop */
         if(module->stack256.top == 0) 
            return (Module_Output) {true, false, false, 0};

         output = module->stack256.data[--module->stack256.top];
         module->stack256.data[module->stack256.top] = 0;
         return (Module_Output) {true, true, true, output};

      case 2: /* peek */
         if(module->stack256.top == 0)
            return (Module_Output) {true, false, false, 0};

         output = module->stack256.data[module->stack256.top];
         return (Module_Output) {true, true, true, output};
   }
}
Module_Output stack256_md2(Module *module, Module_Input_17 input) {
   return (Module_Output) {true, false, false, 0};
}







Module_Output module_md0(Module *module, Module_Input_9 input) {
   switch(module->type) {
      case OUT1: out1_md0(module, input); break;
      case OUT256: out256_md0(module, input); break;
      case IN1: in1_md0(module, input); break;
      case STACK256: stack256_md0(module, input); break;

      default: 
         printf("warning: module type not implemented. type %d\n", module->type);
         break;
   }
}

Module_Output module_md1(Module *module, Module_Input_13 input) {
   switch(module->type) {
      case OUT1: out1_md1(module, input); break;
      case OUT256: out256_md1(module, input); break;
      case IN1: in1_md1(module, input); break;
      case STACK256: stack256_md1(module, input); break;

      default: printf("warning: module type not implemented\n"); break;
   }
}

Module_Output module_md2(Module *module, Module_Input_17 input) {
   switch(module->type) {
      case OUT1: out1_md2(module, input); break;
      case OUT256: out256_md2(module, input); break;
      case IN1: in1_md2(module, input); break;
      case STACK256: stack256_md2(module, input); break;

      default: printf("warning: module type not implemented\n"); break;
   }
}







void file_read_word(FILE *file, char *buffer, int buffer_c) {
   char c;

   while((c = fgetc(file)) != EOF) {
      if(c != ' ' && c != '\t' && c != '\n') break;
   }

   int read_count = 0;
   buffer[read_count++] = c;

   while((c = fgetc(file)) != EOF) {
      if(c == ' ' || c == '\t' || c == '\n') break;
      buffer[read_count++] = c;
      if(read_count >= buffer_c - 1) break;
   }
   buffer[read_count] = '\0';
}

Module **module_array_create(const char *config_file) {
   FILE *file = fopen(config_file, "r");
   if(!file) {
      printf("failed to open module config file\nit should be located in the current directory and be called '%s'\n", config_file);
      return NULL;
   }

   char buffer[256];

   const char *module_name[] = {
      "out1",
      "out256",
      "stack256",
      "in1"
   };
   Module *(*module_create[])(void) = {
      out1_create,
      out256_create,
      stack256_create,
      in1_create
   };

   Module **module_a = malloc(sizeof(*module_a) * 16);
   if(!module_a) return NULL;


   for(int i=0; i<16; i++) {
      file_read_word(file, buffer, sizeof(buffer));

      module_a[i] = NULL;
      for(int j=0; j<sizeof(module_name)/sizeof(*module_name); j++) {
         if(strcmp(buffer, module_name[j]) == 0) {
            module_a[i] = module_create[j]();
            break;
         }
      }
      if(module_a[i] == NULL) module_a[i] = no_module_create();
   }

   return module_a;
}
