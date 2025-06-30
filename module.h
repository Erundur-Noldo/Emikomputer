#ifndef MODULE_H
#define MODUlE_H




typedef struct Module_Input_9 Module_Input_9;
struct Module_Input_9 {
   bool activate;
   u8 data;
};

typedef struct Module_Input_13 Module_Input_13;
struct Module_Input_13 {
   bool activate;
   u8 data;
   u8 argument; /* only the four least significant bits used */
};

typedef struct Module_Input_17 Module_Input_17;
struct Module_Input_17 {
   bool activate;
   u8 data;
   u8 argument[2]; /* only the four least significant bits used */
};

typedef struct Module_Output Module_Output;
struct Module_Output {
   bool finish;
   bool success;
   bool write;
   u8 data;
};







typedef union Module Module;
typedef struct No_Module No_Module;


typedef struct Out1 Out1;
typedef struct Out256 Out256;

typedef struct In1 In1;

typedef struct Stack256 Stack256;



typedef enum Module_Type Module_Type;
enum Module_Type {
   NO_MODULE,

   OUT1, 
   OUT256, 

   IN1, 

   STACK256,

};

struct No_Module {
   Module_Type type;
};


/* OUT1 */
/*
md0:
   outputs the byte communicated (in the emulator, prints to the terminal)
md1:
   nothing
md2:
   nothing
*/
struct Out1 {
   Module_Type type;
};
Module *out1_create();
Module_Output out1_md0(Module *module, Module_Input_9 input);
Module_Output out1_md1(Module *module, Module_Input_13 input);
Module_Output out1_md2(Module *module, Module_Input_17 input);




/* OUT256 */
/*
a 16x16 monochrome screen

md0:
   yyyyxxxx is the location of the pixel
   swap the pixel
md1:
   yyyyxxxx is the location of the pixel
   set the pixel to black if argument is 0, to white otherwise
md2:
   nothing
*/
struct Out256 {
   Module_Type type;
   bool pixel[256];
};
Module *out256_create();
Module_Output out256_md0(Module *module, Module_Input_9 input);
Module_Output out256_md1(Module *module, Module_Input_13 input);
Module_Output out256_md2(Module *module, Module_Input_17 input);



/* IN1 */
/*
md0: requests one byte from the user and returns that to the CPU
md1: nothing
md2: nothing
*/ 
struct In1 {
   Module_Type type;
};
Module *in1_create();
Module_Output in1_md0(Module *module, Module_Input_9 input);
Module_Output in1_md1(Module *module, Module_Input_13 input);
Module_Output in1_md2(Module *module, Module_Input_17 input);



/* STACK256 */
/*
md0: 
   perform action determined by mode
md1:
   arg = 0: set mode to 'push'
   arg = 1: set mode to 'pop'
   arg = 2: set mode to 'peek'
   arg = 4: push once
   arg = 5: pop once
   arg = 6: peek once
md2: nothing
*/
struct Stack256 {
   Module_Type type;
   char mode;
   int top;
   unsigned char data[256];
};
Module *stack256_create();
Module_Output stack256_md0(Module *module, Module_Input_9 input);
Module_Output stack256_md1(Module *module, Module_Input_13 input);
Module_Output stack256_md2(Module *module, Module_Input_17 input);



union Module {
   Module_Type type;

   No_Module no_module;

   Out1 out1;
   Out256 out256;

   In1 in1;

   Stack256 stack256;
};

Module_Output module_md0(Module *module, Module_Input_9 input);
Module_Output module_md1(Module *module, Module_Input_13 input);
Module_Output module_md2(Module *module, Module_Input_17 input);



Module **module_array_create(const char *config_file);



#endif /* MODULE_H */