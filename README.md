amazing emulator and assembler for my invented CPU infrastructure :3



TO COMPILE THE EMULATOR:
gcc -o emulator/main emulator/main.c emulator/cpu.c emulator/im.c emulator/module.c

TO COMPILE THE ASSEMBLER:
gcc -o assembler/assembler assembler/assembler.c

TO ASSEMBLE A PROGRAM FROM BINARY:
./assembler/assembler ./programs/[name of program]

TO RUN A PROGRAM:
./emulator/main out
./emulator/main out r   ((prints the registers after running the program))
./emulator/main out i   ((prints each performed instruction))
./emulator/main out a   ((prints both registers and instructions))


the infrastructure is explained in EMIKOMPUTER_6_1_DEFINITION
the assembly language is explained in EMIKASSEMBLY_6_1_DEFINITION


probably rights to use but not to earn money off of it?? and only to share if you give me credits :(

         .=VVV\     /MMM=.
       /VVVVVVVV===NMWNMWMWA
      /VVVVEEEVVW|ENMWNMMN||A
     /vvEVVVVEVVW|ENWNMNWM//MA
    /VvVVVVVEVVVW|EWWMMWMM||NMA
   /VvVVVVEVVVVVW|EMWMMWWM=NMWMA
  /VEEVVVVVVEVVVW|EMMMMM//WNMWMNA
 /VvVVVVVEVVVVVVW|EMMMM//NMWMWNMWA
/VvVVVP~^~RVVVVWW|KKMMM/P~^~RWWMNMA
VvvVV[______]VVWW|EKMM[______]WNMWN
I\/VV'/VVVVVVVWWW|KEKWMNNWMWNMWMNMW
IIIV/.VVVEEVVWWWM|EKKEWWNMNMWNMWNMW
\ii/.VEVVVVVVMWMW!KEKKMWNMWNMWMNWMV
 -..|VVVVEVVEVMWW!KKEMWNMWNMNWMNWM
 \E:|VVEVVQVVVVWW!KIWMNWNMNMWNMWMV
  /-VVVEVVVEQVVWW!KEMWNMWNMWMWNWM
  /-VVVVVVQVQVVMW|KINWMNWMNWMNMW/
   IVVVVEEVVVVQVW|EWNMNMWNMWNMMV
    VvVVVVVEQEVVW|ANWMWNMNWMMW/
     \VvVIVVVVVVM|AWNWMNMWNW/
       \vVIVVVQVQ|AMWMNWNMN
         vIVIVEVQ|WNMWNMW/
           VvvvVQ|WNMWNV
             VvVQ|WWNV 
               \V|WV
