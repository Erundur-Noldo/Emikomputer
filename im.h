
#ifndef IM_H
#define IM_H




typedef union Im Im;
typedef enum ImType ImType;
enum ImType {
   IM4K,
};


typedef struct Im_Header Im_Header;
struct Im_Header {
   ImType type;
   s32 *cpu_cycles_busy; /* used to edit that */
};



/*
Im4k:

4096 instructions
256 flags containing pointers to instructions
branch depth of 16

the last flag (flag#fff) is reserved to contain 0xfff
and instruction#fff is reserved to contain 0x0 (stp)

when ret instruction is provided, pops the stack and puts 0xff in the 'empty space' created
basically, when returning from the main program, it stops the program in an error state
*/

typedef struct Im4k Im4k;
struct Im4k {
   Im_Header header;
   u8 top;
   u16 return_stack[0x10];
   u16 flag[0x100];
   u8 memory[0x1000];
};

Im *im4k_create(char *source);






union Im {
   Im_Header header;

   Im4k im4k;
};





u8 im_receive(Im *im);
void im_jump(Im *im, u8 pointer_index);
void im_branch(Im *im, u8 pointer_index);
void im_return(Im *im);



#endif /* IM_H */