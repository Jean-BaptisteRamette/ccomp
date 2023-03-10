# chip8-asm

### Comments:
Comments are used this way:

```asm
;; this will be ignored !!
```

### Numeric bases:
Decimal, binary, hexadecimal and octal bases are supported by ccomp:

```c++
255 = 0b11111111 = 0b1111'1111 = 0xFF = 0o377
```
you can use ascii to represent integral types as well
```c++
65 = 'A'
```

### Labels:
Lets you define code sections
```asm
label1:
    ;; some code
    
label2:
    ;; some code
```

### Entry Point:
The entry point of your program must be a main label.
```asm
main:
    ;; entry point code
```


### Constants definitions:
You can declare constants using the "define" keyword:
```c
define number 178  ;; assign value 178 to constant 'number'
```

Constant declarations can be limited to a scope:

```asm
label1:
    define number 178
    add rc, number

label2:
    sub r1, number  ;; ERROR: number not defined here
``` 

### Registers:
#### General Purpose:
The chip-8 defines 16 8-bit general purpose registers. Each register is named from r0 to rf.

#### Special Purpose:
The chip-8 has 4 special purpose registers.

        pc: program counter, 16 bits, hold current instruction address
        sp: stack pointer, points to the top of the stack
        ar: address register, 16 bits, holds memory address
        dt: delay timer, 8-bit used for CPU time management
        st: sound timer, 8-bit used for APU time management

### Instruction Set:
#### Control Flow:

```asm
call 2000      ;; call subroutine at location
ret            ;; returns from a subroutine
jmp [4000]     ;; jumps to address r0 + 4000
jmp 4000       ;; jumps to address 4000
jmp .my_label  ;; jumps to my_label
```

#### Comparisons:

```asm
eq r0, r2   ;; skips next instruction if r0 == r2
eq r0, 35   ;; skips next instruction if r0 == 35
neq r0, r2  ;; skips next instruction if r0 != r2
neq r0, 35  ;; skips next instruction if r0 != 35
```

Examples:
```asm
eq rf, 255  ;; if rf == 255, skips jump to case_not_equal
jmp .case_not_equal
jmp .case_equal
```


#### Maths:
Common operations:
```asm
add r0, r2   ;; adds r2 to register r0, carry flag is set
add rd, 15   ;; adds 15 to register rd, carry flag is not changed
sub rb, r2   ;; r2 is subtracted from rb, carry flag is set
suba r1, r2  ;; sets r1 to r2 minus r1, carry flag is set
```

Bitwise operations:
```asm
or r1, r2   ;; sets r1 to r1 OR r2
and r1, r2  ;; sets r1 to r1 AND r2
xor r1, r2  ;; sets r1 to r1 xor r2
shr r1      ;; stores MSB of r1 to rf then shifts r1 to the right by 1
shl r1      ;; stores LSB of r1 to rf then shifts r1 to the left by 1
```

GPR Manipulation:

```asm
rdump rb     ;; stores each register value from r1 to rb (included) contiguously in memory starting from address I.
rload rb     ;; sores values from memory starting at address I to register r1 to rb (included).

mov re, 0xFF ;; sets re to value 0xFF
mov ra, rd   ;; stores rd into ra
swp ra, rd   ;; swaps registers values
```

SPR Manipulation:
```asm
mov ar, 3456  ;; sets address register to address 3456
mov ar, r1   ;; sets address register to the location of the sprite for the character in r1
add ar, r1   ;; adds r1 to address register, carry flag is not changed

mov dt, r0  ;; sets the delay timer to r0
mov st, r0  ;; sets the delay timer to r0

mov rb, dt  ;; sets rb to dt
```
You cannot directly set the PC register, you must use the jmp instructions

Stack Manipulation:

Screen Manipulation:
```asm
draw rb, ra, N  ;; draws a sprite at coordinates (rb, ra) with height N + 1 pixels
cls             ;; clear screen
```

Keypad Manipulation:
```asm
kpw rd   ;; key press is awaited and stored in rd
eq rb    ;; skips the next instruction of the key stored in rb is pressed
neq rc   ;; skips the next instruction of the key stored in rc is not pressed
```

Random Numbers:
```asm
rand r1, 34  ;; sets r1 to a random value between 0 et 34
```

BCD:
```asm
bcd re  ;; stores BCD representation of re register with the MSB at address I
```

#### Inline opcodes:
    ccomp allows the programmer to put inline raw opcodes in the source file using the raw keyword:

        define OPCODE 0x00E0
        label:
            raw(OPCODE)  ;; clear the screen


### Instruction Table

|    mnemonics   |      opcode     |
|:--------------:|:---------------:|
|   add rX, rY   |       8XY4      |
|   add rX, NN   |       7XNN      |
|   sub rX, rY   |       8XY5      |
|   suba rX, rY  |       8XY7      |
|    or rX, rY   |       8XY1      |
|   and rX, rY   |       8XY2      |
|   xor rX, rY   |       8XY3      |
|     shr rX     |       8XYE      |
|     shl rX     |       8XY6      |
|    rdump rX    |       FX55      |
|    rload rX    |       FX65      |
|   mov rX, NN   |       6XNN      |
|   mov rX, rY   |       8XY0      |
|   swp rX, rD   | ccomp extension |
|   mov ar, NNN  |       ANNN      |
|   mov ar, rX   |       FX29      |
|   add ar, rX   |       FX1E      |
|   mov dt, rX   |       FX15      |
|   mov st, rX   |       FX18      |
| draw rX, rY, N |       DXYN      |
|       cls      |       00E0      |
|   rand rX, NN  |       CXNN      |
|     bcd rX     |       FX33      |
|     kpw rX     |       FX0A      |
|      eq rX     |       EX9E      |
|     neq rX     |       EXA1      |
|       ret      |       00EE      |
|     jmp NNN    |       1NNN      |
|   jmp .label   |       1NNN      |
|    call NNN    |       2NNN      |
|    jmp [NNN]   |       BNNN      |
|    eq rX, NN   |       3XNN      |
|   neq rX, NN   |       4XNN      |
|    eq rX, rY   |       5XY0      |
|   neq rX, rY   |       9XY0      |
|   mov rX, dt   |       FX07      |
