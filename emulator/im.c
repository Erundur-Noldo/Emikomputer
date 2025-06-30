

/* IM4K */

#include "main.h"
#include "im.h"


Im *im4k_create(char *source) {
   FILE *file = fopen(source, "rb");
   if(!file) return NULL;

   Im *im = malloc(sizeof(*im));
   if(!im) {
      fclose(file);
      return NULL;
   }

   im->im4k = (Im4k) {
      .header = {
         .type = IM4K,
         .cpu_cycles_busy = NULL /* this will be assigned when a cpu is created with this im */
      },

      .return_stack = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
   };

   int read_bytes;
   read_bytes = fread(im->im4k.memory, 1, sizeof(im->im4k.memory), file);
   assert(read_bytes == sizeof(im->im4k.memory));

   read_bytes = fread(im->im4k.flag, 2, 0x100, file);
   assert(read_bytes == 0x100);

   fclose(file);
   return im;
}


u8 im_receive(Im *im) {
   (*im->header.cpu_cycles_busy)++;
   return im->im4k.memory[im->im4k.return_stack[0]++];
}

void im_jump(Im *im, u8 pointer_index) {
   im->im4k.return_stack[0] = im->im4k.flag[pointer_index];
}

void im_branch(Im *im, u8 pointer_index) {
   for(int i=15; i>0; i--)
      im->im4k.return_stack[i] = im->im4k.return_stack[i-1];

   im->im4k.return_stack[0] = im->im4k.flag[pointer_index];
}

void im_return(Im *im) {
   for(int i=0; i<15; i++) {
      im->im4k.return_stack[i] = im->im4k.return_stack[i+1];
      im->im4k.return_stack[15] = 0xfff;
   }
}
