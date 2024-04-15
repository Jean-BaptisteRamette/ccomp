# chasm
### _Custom assembly language for the chip-8 and SuperChip-48 virtual machines_

This project is an assembler generating code according to the CHIP-8 ISA.
It allows programmer to write code easily, and can be used as a way to test their emulators.

## Summary
- [I. Features](#i---features)
- [II. What does it look like ?](#ii---example-program)
- [III. Command line options](#iii---command-line-options)
- [IV. Language specifications](#iv---language-specifications)
- [V. Instruction reference](#v---mnemonics-and-opcodes-mapping)
- [VI. Contributing](#vi---contributing)

## I - Features

- Nice and intuitive syntax for easy writing
- Full ISA support
- Support for SuperCHIP-48 extensions
- Scoped symbols
- Constant declarations
- Multiple numerical bases supported
- Syntactic sugar and pseudo-instructions
- Inline opcodes support for unsafe code
- Bitshift instructions support both single/two operand(s)
- Easily modifiable syntax through source code


## II - Example program

```asm
.main:
    ;; TODO
```

## III - Command line options

```
Usage:
  chasm [OPTION...]

  -h, --help                    Show help message
      --in arg                  chasm source file to assemble
      --out arg                 The generated machine code output file path
                                (default: out.c8c)
      --hex [=arg(=4)]          Hexdumps the generated machine code,
                                argument is the amount of opcodes per line
      --symbols [=arg(=out.c8s)]
                                Generate a file with symbols location in
                                memory/machine code
      --relocate arg            Address in which the binary is supposed to
                                be loaded (default: 0x200)
      --super                   Specify the target ISA to be the SUPER-CHIP
                                and removes warning when using non CHIP-8
                                instructions
```

## IV - Language Specifications

1. [Comments](#1-comments)
2. [Numeric literals](#2-numeric-literals)
3. [Registers](#3-registers)
4. [Arithmetical and logical operations](#4-arithmetical-and-logical-operations)
5. [Control flow](#5-control-flow)
6. [Procedures](#6-procedures)
7. [Constants](#7-constants)
8. [General purpose register manipulation](#8-general-purpose-registers-manipulation)
9. [Special purpose register manipulation](#9-special-purpose-registers-manipulation)
10. [Inline opcodes](#10-inline-opcodes)
11. [I/O](#11-io)
12. [Sprites](#12-sprites)
13. [Others](#13-others)

### 1. Comments
Use the `;;` characters to write comments

```asm
;; this text will be ignored !
```
### 2. Numeric literals
There are multiple ways to write literals in chasm, as all usual bases are supported:
```c++
255                ;; decimal
0b11111111         ;; binary
0b1111'1111        ;; binary with delimiter
0xFF               ;; hexadecimal
0o377              ;; octal
```
You can also use ascii to represent integral values
```
'A'  ;; evaluates to integer 65
```
>Note: You can only manipulate values in range [0-255], as it is restricted by the ISA

### 3. Registers

The chip-8 has 16 registers, one byte each. In chasm, they are named from `r0` to `rf`. However, please be careful when using the `rf` register, as it is used by some instruction as an indicator.

### 4. Arithmetical and logical operations
The chip-8 only defines addition and subtraction instructions, as well as the regular bitwise operations
##### Arithmetical
```asm
add r0, r2   ;; adds r2 to register r0, carry flag is set
add rd, 15   ;; adds 15 to register rd, carry flag is not changed
sub rb, r2   ;; r2 is subtracted from rb, carry flag is set
suba r1, r2  ;; sets r1 to r2 minus r1, carry flag is set
```
> Note: registers in this example can be changed to any other general purpose register
##### Logical
```asm
or r1, r2   ;; sets r1 to r1 OR r2
and r1, r2  ;; sets r1 to r1 AND r2
xor r1, r2  ;; sets r1 to r1 xor r2
shr r1      ;; stores MSB of r1 to rf then shifts r1 to the right by 1
shr r1, r2  ;; same as above but r2 is moved into r1 first
shl r1      ;; stores LSB of r1 to rf then shifts r1 to the left by 1
shl r1, r2  ;; same as above but r2 is moved into r1 first
```
> Note: registers in this example can be changed to any other general purpose register

### 5. Control flow
You are quite probably going to need to jump or conditionally execute different code paths, the chip-8 offers you basic instructions to implement this
#### Unconditional jumps
```asm
jmp [0x32]      ;; jumps to the address (r0 + 0x32)
jmp @my_label   ;; jumps to label
```

#### Conditional jumps
The chip-8 uses a "skip next if ..." rather than a "jump if ..." idiom:
```asm
se r0, r2      ;; skips next instruction if r0 == r2
se r0, 32      ;; skips next instruction if r0 == 32
sne r0, r2     ;; skips next instruction if r0 != r2
sne r0, 35     ;; skips next instruction if r0 != 35
```
This means you can write conditional jumps this way
```asm
se r2, 0xCC
jmp @case_not_equal
jmp @case_equal
```
However, this happens to be trickier with `>` and `<` comparisons, as there is only one existing jump instruction. For example, in x86, you would do something like this...
```asm
cmp cx, 0xCC     ;; compare cx and 0xCC and store result in indicator flags
ja case_above    ;; jump to label if cx > 0xCC
...
```
...which becomes the following in chip-8 assembly:
```asm
mov r0, 0xCC         ;; move immediate in temporary register (now cloberred)
suba r0, r2          ;; compare r0 to r2 by subtracting (r0 = r2 - r0), (vf = r2 > r0)
se rf, 0             ;; if rf == 1 then r2 > r0
jmp @case_above      ;; jump to label if rf == 1
...
```
#### Labels
To define code location to which the processor can jump, you need to define labels
```
	jmp .label_name

.label_name:
	...code...	
``` 
There is a reserved label name called `.main`, which is your program entry-point, and that any chasm program must define

### 6. Procedures
chasm allows the programmer to define procedures

```asm
proc my_proc 
	xor r0, r0

.done:
	ret            ;; return from procedure to sne instruction
endp my_proc

.main:
	call $my_proc
	sne r0, 0
```

### 7. Constants
You can declare constants using the `define` keyword:
```asm
define MAX_HEALTH 100  ;; assign constant value 100 to identifier 'MAX_HEALTH'
```
Constant declarations can be limited to a procedure's scope:
```asm
proc my_proc
	define value 0
	mov rd, value ;; rd will be 0
	ret
endp my_proc

.main:
	define value 255
	mov rd, value ;; rd will hold value 255
```
However, you cannot redefine a constant already defined in an outer scope
```asm
define value 0

proc my_proc
	define value 0  ;; invalid
	mov rd, value
	ret
endp my_proc
```


### 8. General purpose registers manipulation

```asm  
rdump rb     ;; stores each register value from r1 to rb (included) contiguously in memory starting from address I.  
rload rb     ;; sores values from memory starting at address I to register r1 to rb (included).  
  
mov re, 0xFF ;; sets re to value 0xFF  
mov ra, rd   ;; stores rd into ra  
swp ra, rd   ;; swaps registers values  
```  

### 9. Special purpose registers manipulation

```asm  
mov ar, 3456  ;; sets address register to address 3456  
mov ar, r1    ;; sets address register to the location of the sprite for the character in r1  
add ar, r1    ;; adds r1 to address register, carry flag is not changed  
  
mov dt, r0    ;; sets the delay timer to r0  
mov st, r0    ;; sets the delay timer to r0  
  
mov rb, dt    ;; sets rb to dt  
```  
> Note: You cannot directly set the instruction pointer register (PC), you must use the `jmp`/`call` and `ret` instructions.


### 10. Inline opcodes

chasm allows the programmer to put inline raw opcodes in the source file using the raw keyword:

```asm  
.label:  
    define OPCODE 0x00E0  
    raw(OPCODE)  ;; clear the screen  
```

This is particularly useful if you want to generate code that is not allowed by the assembler

### 11. I/O

Drawing
```asm  
draw rb, ra, N  ;; draws a sprite at coordinates (rb, ra) with a height of N + 1 digits
cls             ;; clear screen  
```  

Input key
```asm  
wkey rd   ;; key press is awaited and stored in rd  
ske rb    ;; skips the next instruction if the key stored in rb is pressed  
skne rc   ;; skips the next instruction if the key stored in rc is not pressed  
```  

### 12. Sprites
You can declare sprites using the `sprite` keyword:
```asm
sprite my_sprite [ 
           0b0'0'0'1'1'0'0'0,
           0b0'0'1'1'1'1'0'0,
           0b0'1'1'1'1'1'1'0,
           0b1'1'0'1'1'0'1'1,
           0b1'1'1'1'1'1'1'1,
           0b0'0'1'0'0'1'0'0,
           0b0'1'0'1'1'0'1'0,
           0b1'0'0'0'0'0'0'1,
       ]
```
Sprites must be at most 15 lines high, meaning, 15 8-bits integers between the brackets.

You can make AR (also known as I register) point to it
```asm
mov ar, #my_sprite
```
And draw them
```asm
proc DrawSprite
    mov ar, #my_sprite
    draw r0, r1, #my_sprite
    ret
endp DrawSprite

proc DrawSprite4
    mov ar, #my_sprite
    draw r0, r1, 4
    ret
endp DrawSprite4

.main:
    ;; display full sprite at position (0, 4)
    mov r0, 0
    mov r1, 4
    call $DrawSprite
    
    ;; display only the first 4 lines of the sprite
    call $DrawSprite4
```

### 13. Others

Random number generator 
```asm  
rand r1, 34  ;; sets r1 to a random value between 0 et 34  
```  

Binary-coded-decimal
```asm  
bcd re  ;; stores BCD representation of re register with the MSB at address I  
```  

## V - Mnemonics and opcodes mapping

Reference for the instructions mnemonics and what machine code they produce once assembled

|    mnemonics     |      opcodes       |
|:----------------:|:------------------:|
|    mov rX, NN    |        6XNN        |
|    mov rX, rY    |        8XY0        |
|   mov ar, NNN    |        ANNN        |
|    mov ar, rX    |        FX29        |
|    mov dt, rX    |        FX15        |
|    mov st, rX    |        FX18        |
|    mov rX, dt    |        FX07        |
|    swp rX, rD    | pseudo-instruction |
|    jmp @label    |        1NNN        |
|    jmp [NNN]     |        BNNN        |
| call $subroutine |        2NNN        |
|       ret        |        00EE        |
|    se rX, NN     |        3XNN        |
|    sne rX, NN    |        4XNN        |
|    se rX, rY     |        5XY0        |
|    sne rX, rY    |        9XY0        |
|    add rX, rY    |        8XY4        |
|    add rX, NN    |        7XNN        |
|    add ar, rX    |        FX1E        |
|    sub rX, rY    |        8XY5        |
|   suba rX, rY    |        8XY7        |
|    or rX, rY     |        8XY1        |
|    and rX, rY    |        8XY2        |
|    xor rX, rY    |        8XY3        |
|      shr rX      |        8XYE        |
|      shl rX      |        8XY6        |
|    shr rX, rY    |        8XYE        |
|    shl rX, rY    |        8XY6        |
|     rdump rX     |        FX55        |
|     rload rX     |        FX65        |
|   rand rX, NN    |        CXNN        |
|      bcd rX      |        FX33        |
|     wkey rX      |        FX0A        |
|      ske rX      |        EX9E        |
|     skne rX      |        EXA1        |
|  draw rX, rY, N  |        DXYN        |
|       cls        |        00E0        |

> Note: `swp rX, rD` is a chasm extension generating the appropriate code needed to swap 2 registers.


## VI - Contributing

Contributions are absolutely welcome !
They can come as bug-reports, bug-fixes, features, documentation or just ideas, that I will be pleased to integrate to chasm !

### 1. Bug-reports
Please provide a minimal reproducible example of the bug, and explain it the best you can if you want it to be fixed quickly.

### 2. Pull-requests
I am sure this project is suitable for beginners to write some of their first PRs and that
there are good-first-issues here and there. If you have any question about the code base or difficulty on an issue you want
to fix, do not hesitate to contact me.

Please, state clearly what you added, and follow the already existing code style.

Finally, more important PRs will not be merged if they do not come with unit-tests associated.


## License

MIT

