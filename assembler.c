
#include <stdio.h>
#include <string.h>
#include <ctype.h>


#define MAX_NAME_LENGTH 64

char char_to_u8(char c) {
   if('0' <= c && c <= '9') return c - '0';
   if('a' <= c && c <= 'f') return c - 'a' + 10;
   if('A' <= c && c <= 'F') return c - 'A' + 10;
   return -1; /* incorrect input */
}

char u8_to_char(char c) {
   if(0x0 <= c && c <= 0x9) return c + '0';
   if(0xa <= c && c <= 0xf) return c + 'a' - 10;
   return -1; /* incorrect input */
}


typedef struct Module Module;
struct Module {
   char identifier[MAX_NAME_LENGTH];
};


typedef struct Label Label;
struct Label {
   char identifier[MAX_NAME_LENGTH];
};

char fgetc_skip_whitespace(FILE *file) {
   char c;
   while((c = fgetc(file)) != EOF) {
      if(c != ' ' && c != '\t' && c != '\n') break;
   }
   return c;
}

void file_skip_whitespace(FILE *file) {
   char c;
   while((c = fgetc(file)) != EOF) {
      if(c != ' ' && c != '\t' && c != '\n') break;
   }
   fseek(file, -1, SEEK_CUR);
}

void file_skip_line(FILE *file) {
   char c;
   while((c = fgetc(file)) != EOF) {
      if(c == '\n') break;
   }
}

void file_read_word(FILE *file, char *buffer, int buffer_c) {
   file_skip_whitespace(file);
   char c;
   int read_count = 0;
   while((c = fgetc(file)) != EOF) {
      if(c == ' ' || c == '\t' || c == '\n') break;
      buffer[read_count++] = c;
      if(read_count >= buffer_c - 1) break;
   }
   buffer[read_count] = '\0';
}



int get_mod_and_label(FILE *file, Module *module_a, Label *label_a) {
   char c;
   int label_c = 0;
   while((c = fgetc(file)) != EOF) {
      if(c == '~') {
         file_read_word(file, label_a[label_c++].identifier, MAX_NAME_LENGTH);
      }

      if(c == '#') {
         const char header[] = "module";
         for(int i=0; i<sizeof(header)-1; i++) {
            if(fgetc(file) != header[i]) return 1;
         }

         file_skip_whitespace(file);
         char port = char_to_u8(fgetc(file));
         file_skip_whitespace(file);
         file_read_word(file, module_a[port].identifier, MAX_NAME_LENGTH);
      }
   }

   return 0;
}



char read_digit(FILE *file) {
   file_skip_whitespace(file);
   return char_to_u8(fgetc(file));
}

char read_register(FILE *file) {
   file_skip_whitespace(file);

   char instruction[4];
   instruction[0] = fgetc(file);
   instruction[1] = fgetc(file);
   instruction[2] = fgetc(file);
   instruction[3] = '\0';

   if(instruction[0] == 'r' && instruction[1] == 'g') {
      return char_to_u8(instruction[2]);
   }


   const char instruction_str[][4] = {
      "acu", "opr", "mdi", "mdo",
      "",    "",    "",    "",
      "",    "",    "",    "",
      "",    "ctr", "flg", "zro"
   };

   for(char i=0; i<16; i++) {
      if(strcmp(instruction, instruction_str[i]) == 0)
         return i;
   }

   return -1;
}

char read_alu_mode_from_string(char *instruction) {
   const char instruction_str[][4] = {
      "add", "sub", "inc", "dec",
      "bng", "orr", "and", "xor",
      "lsh", "rsh", "lrt", "rrt",
      "ang", "lng", "log", "pas"
   };

   for(char i=0; i<16; i++) {
      if(strcmp(instruction, instruction_str[i]) == 0)
         return i;
   }

   return -1;
}
char read_alu_mode(FILE *file) {
   file_skip_whitespace(file);
   char instruction[4];
   instruction[0] = fgetc(file);
   instruction[1] = fgetc(file);
   instruction[2] = fgetc(file);
   instruction[3] = '\0';

   return read_alu_mode_from_string(instruction);
}


char read_flag(FILE *file) {
   file_skip_whitespace(file);
   char instruction[4];
   int inverse;

   char c = fgetc(file);
   inverse = c == '!';
   if(inverse) file_skip_whitespace(file);

   instruction[0] = inverse ? fgetc(file) : c;
   instruction[1] = fgetc(file);
   instruction[2] = fgetc(file);
   instruction[3] = '\0';


   const char instruction_str[][4] = {
      "tru", "eqq", "les", "grt",
      "odd", "los", "mdf", "sof",
      "fls", "neq", "geq", "leq",
      "evn", "",    "",    ""
   };

   for(char i=0; i<16; i++) {
      if(strcmp(instruction, instruction_str[i]) == 0)
         return (0x8 * inverse) ^ i;
   }

   return -1;

}

char read_module(FILE *file, Module *module_a) {
   char buffer[MAX_NAME_LENGTH];
   file_read_word(file, buffer, MAX_NAME_LENGTH);

   for(int i=0; i<16; i++) {
      if(strcmp(buffer, module_a[i].identifier) == 0)
         return i;
   }

   return -1;
}

/* returns an int because every char is a valid return, and an error state is still needed */
int read_label(FILE *file, Label *label_a) {

   char buffer[MAX_NAME_LENGTH];
   file_read_word(file, buffer, MAX_NAME_LENGTH);

   for(int i=0; i<256; i++) {
      if(strcmp(buffer, label_a[i].identifier) == 0)
         return i;
   }

   return -1;
}


#define INSTR_TO_INT(str) ( (int) (((int) str[0]<<16) | ((int) str[1]<<8) | ((int) str[2])))
/*
stp: 0x737470
wrt: 0x777274
cpy: 0x637079
swp: 0x737770
cmp: 0x636d70
jmp: 0x6a6d70
brn: 0x62726e
ret: 0x726574
cnt: 0x636e74
*/
int read_line(FILE *file, Module *module_a, Label *label_a, char *output) {
   char c = fgetc(file);

   /* first, we skip the module declarations */
   if(c == '\n' || c == '#') {
      fseek(file, 0, SEEK_SET);
      while((c = fgetc(file)) != EOF) {
         if(c == '\n') {
            c = fgetc(file);
            if(c == '#') continue;
            if(c == '\n') {
               fseek(file, -1, SEEK_CUR);
               continue;
            }
            /* stop with skipping lines */
            fseek(file, -1, SEEK_CUR);
            break;
         }
      }
   }

   int output_c = 0;
   int label_c = 0;
   while((c = fgetc_skip_whitespace(file)) != EOF) {
      output[output_c++] = 0x10; /* we're using this as a newline for a bit because 0xa is also newline :v */



      if(c == '#') file_skip_line(file);
      if(c == '~') {
         int index = read_label(file, label_a);
         if(index == -1) return -1;

         output[output_c++] = '~';
         output[output_c++] = (index >> 4) & 0x0f;
         output[output_c++] = index & 0x0f;

         fseek(file, -1, SEEK_CUR); /* very scuffed bugfix */
         file_skip_line(file);
         continue;
      }



      /* the following is pretty cursed but it allows me to use a switch */
      char instruction[4];
      instruction[0] = c;
      instruction[1] = fgetc(file);
      instruction[2] = fgetc(file);
      instruction[3] = '\0';


      int found = 1;

      int label;
      char origin, high, low, target, mode, flag;
      switch(INSTR_TO_INT(instruction)) {
         case(0x737470): /* stp */
            output[output_c++] = 0x0;
            break;

         case(0x777274): /* wrt */
            output[output_c++] = 0x1;
            origin = read_register(file);
            high = read_digit(file);
            low = read_digit(file);

            if(origin == -1 || high == -1 || low == -1) {
               return -1;
            }

            output[output_c++] = origin;
            output[output_c++] = high;
            output[output_c++] = low;
            break;

         case(0x637079): /* cpy */
            output[output_c++] = 0x2;
            origin = read_register(file);
            target = read_register(file);

            if(origin == -1 || target == -1) {
               return -1;
            }
            output[output_c++] = origin;
            output[output_c++] = target;
            break;

         case(0x737770): /* swp */
            output[output_c++] = 0x3;
            origin = read_register(file);
            target = read_register(file);

            if(origin == -1 || target == -1) {
               return -1;
            }
            output[output_c++] = origin;
            output[output_c++] = target;
            break;

         case(0x636d70): /* cmp */
            output[output_c++] = 0x5;
            mode = read_alu_mode(file);

            if(mode == -1) {
               return -1;
            }

            output[output_c++] = mode;
            break;


         case(0x6a6d70): /* gto */
            output[output_c++] = 0x8;
            flag = read_flag(file);
            label = read_label(file, label_a);

            if(flag == -1 || label == -1) {
               return -1;
            }

            high = (label >> 4) & 0x0f;
            low = label & 0x0f;

            output[output_c++] = flag;
            output[output_c++] = high;
            output[output_c++] = low;
            break;


         case(0x62726e): /* brn */
            output[output_c++] = 0x9;
            flag = read_flag(file);
            label = read_label(file, label_a);

            if(flag == -1 || label == -1) {
               return -1;
            }

            high = (label >> 4) & 0x0f;
            low = label & 0x0f;

            output[output_c++] = flag;
            output[output_c++] = high;
            output[output_c++] = low;
            break;


         case(0x726574): /* ret */
            output[output_c++] = 0xa;
            break;

         case(0x636e74): /* cnt */
            output[output_c++] = 0xb;
            label = read_label(file, label_a);

            if(label == -1) {
               return -1;
            }

            high = (label >> 4) & 0x0f;
            low = label & 0x0f;

            output[output_c++] = high;
            output[output_c++] = low;
            break;


         default: 
            found = 0;
            break;
      }

      if(found) continue;



      /* check alu first, then module stuff */
      char alu_mode = read_alu_mode_from_string(instruction);
      if(alu_mode != -1) {
         output[output_c++] = 0x4;
         output[output_c++] = alu_mode;
         continue;
      }

      /* if not found, we've still skipped three characters */
      // fseek(file, -1, SEEK_CUR);


      fseek(file, -3, SEEK_CUR);
      char module_index = read_module(file, module_a);
      if(module_index == -1) return -1;

      char arg_1, arg_1_next, arg_2, arg_2_next; /* these are optional, we have to check for them and possibly backtrack */
      long starting_pos = ftell(file); /* for backtracking */

      /* there are three options:

      if there is no argument:
         arg_1 followed by character
         arg_2 followed by character

      if there is one argument:
         arg_1 followed by whitespace
         arg_2 followed by character

      if there are two arguments:
         arg_1 followed by either
         arg_2 followed by whitespace
      */

      arg_1 = fgetc_skip_whitespace(file);
      arg_1_next = fgetc(file);

      arg_2 = isspace(arg_1_next) ? fgetc_skip_whitespace(file) : arg_1_next;
      arg_2_next = fgetc(file);



      if(isspace(arg_2_next)) { /* 2 arguments */
         if(char_to_u8(arg_1) == -1) return -1;
         if(char_to_u8(arg_2) == -1) return -1;

         output[output_c++] = 0xf;
         output[output_c++] = module_index;
         output[output_c++] = char_to_u8(arg_1);
         output[output_c++] = char_to_u8(arg_2);

         continue;
      }
      else if(isspace(arg_1_next)) { /* 1 argument */
         if(char_to_u8(arg_1) == -1) return -1;

         output[output_c++] = 0xe;
         output[output_c++] = module_index;
         output[output_c++] = char_to_u8(arg_1);

         fseek(file, starting_pos, SEEK_SET);
         fgetc_skip_whitespace(file);
         continue;
      }
      else { /* 0 arguments */
         output[output_c++] = 0xd;
         output[output_c++] = module_index;

         fseek(file, starting_pos, SEEK_SET);
         continue;
      }



      /* if we get here, parsing failed */
      return -1;

   }

   return output_c;
}

int remove_comments(FILE *in, FILE *out) {
   char c;
   int comment_depth = 0;
   while((c = fgetc(in)) != EOF) {
      if(comment_depth < 0) return 1;

      if(c == '/') {
         char next = fgetc(in);
         if(next != '-') return 1;
         comment_depth++;
         continue;
      }

      if(c == '-') {
         char next = fgetc(in);
         if(next != '/') return 1;
         comment_depth--;
         continue;
      }

      if(comment_depth == 0) fputc(c, out);
   }
   return 0;
}

char scuffed_translation(char a) {
   if(a == 0x10) return '\n';
   if(a == '~') return '~';
   return u8_to_char(a);
}



int hex_to_bin(FILE *in) {


   FILE *out = fopen("./out", "w");
   if(!out) return 1;

   short flag[0x100];
   for(int i=0; i<0x100; i++) flag[i] = 0;

   char c;
   int depth = 0;
   bool in_comment = false;
   while((c = fgetc(in)) != EOF) {
      if(c == '\n') {
         in_comment = false;
         continue;
      }
      if(c == ' ') continue;
      if(c == '-') in_comment = true;

      if(in_comment) continue;

      if(c == '~') {
         char c_2 = char_to_u8(fgetc(in));
         char c_3 = char_to_u8(fgetc(in));

         if(c_2 == -1 || c_3 == -1) {
            printf("flag incorrectly assigned\n");
            fclose(in);
            fclose(out);
            return 1;
         }

         flag[(c_2 << 4) | c_3] = depth;
         continue;
      }

      fputc(char_to_u8(c), out);
      depth++;
   }


   while(depth++ < 0x1000) {
      fputc(0, out);
   }

   fwrite(flag, 2, 0x100, out);

   fclose(out);

   return 0;

}




int main(int argc, char *args[]) {
   int failure;

   if(argc != 2) {
      printf("please provide one (1) source file\n");
      return 1;
   }
   FILE *input = fopen(args[1], "r");
   if(!input) {
      printf("failed to load source file. Are you sure it exists??\n");
      return 1;
   }

   FILE *input_no_comments = fopen("./no_comments", "w+");
   if(!input_no_comments) {
      printf("failed to create temporary file\n");
      fclose(input);
      return 1;
   }
   failure = remove_comments(input, input_no_comments);
   fclose(input);
   if(failure) {
      fclose(input_no_comments);
      printf("something went wrong trying to remove comments :(\n");
      return 1;
   }
   fseek(input_no_comments, 0, SEEK_SET);

   Module module[16];
   for(int i=0; i<16; i++) {
      module[i].identifier[0] = '\0';
   }

   Label label[256];
   for(int i=0; i<256; i++) {
      label[i].identifier[0] = '\0';
   }

   failure = get_mod_and_label(input_no_comments, module, label);
   if(failure) {
      printf("error parsing labels or modules\n");
      return 1;
   }

   // for(int i=0; i<16; i++) {
   //    if(module[i].identifier[0] != '\0')
   //       printf("%d: %s\n", i, module[i].identifier);
   // }

   // for(int i=0; i<256; i++) {
   //    if(label[i].identifier[0] != '\0')
   //       printf("%d: %s\n", i, label[i].identifier);
   // }


   fseek(input_no_comments, 0, SEEK_SET);



   char program[8192]; /* twice the max size to account for newlines */
   int chars_written = read_line(input_no_comments, module, label, program);

   char c;
   const int space_around = 10;
   if(chars_written == -1) {
      printf("failed to assemble: ");
      fseek(input_no_comments, -space_around - 3, SEEK_CUR);
      for(int i=0; i<space_around; i++) printf("%c", (c = fgetc(input_no_comments)) == '\n' ? ' ' : c);
      printf("\e[1;31m>>>");
      for(int i=0; i<3; i++) printf("%c", (c = fgetc(input_no_comments)) == '\n' ? ' ' : c);
      printf("<<<\e[0m");
      for(int i=0; i<space_around; i++) printf("%c", (c = fgetc(input_no_comments)) == '\n' ? ' ' : c);
      printf("\n");

   }

   fclose(input_no_comments);



   FILE *output_hex = fopen("./out_hex", "w+");
   if(!output_hex) {
      printf("failed to open temp file :(\n");
      return 1;
   }
   for(int i=0; i<chars_written; i++) 
      program[i] = scuffed_translation(program[i]);
   fwrite(program, 1, chars_written, output_hex);

   fseek(output_hex, 0, SEEK_SET);
   failure = hex_to_bin(output_hex);
   if(failure) {
      printf("failed to put the hex into bin :(((\n");
      return 1;
   }
   fclose(output_hex);

   printf("assembled successfully ^>^\n");
   return 0;
}