/*
 ============================================================================
 Name        : Arithmetic Machine
 Author      : UBC Launchpad
 Version     : 1.0
 Copyright   :
 Description : Stack-based virtual machine that performs simple arithmetic.
 ============================================================================
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define STACK_SIZE   256                                 // maximum number of values on the stack
#define PUSH(vm, v)  (vm->stack[++vm->sp] = v)           // push value onto stack
#define POP(vm)      (vm->stack[vm->sp--])               // get value from top of stack
#define NCODE(vm)    (vm->code[vm->pc++])                // get next bytecode
#if LITTLE_ENDIAN
#define JMP(vm)      {\
    memcpy(&vm->pc, vm->code + vm->pc, 4);\
    convertToLittleEndian(&vm->pc, sizeof(int));\
}
#else
#define JMP(vm)      (memcpy(&vm->pc, vm->code + vm->pc, 4))
#endif

/* all opcodes that will be implemented. */
enum opcodes {
    HALT      = 0x00, // halt
    /* in case you can't get DCONST working, you can still try things using these: */
    DCONST_M1 = 0x0A, // push -1.0 onto stack
    DCONST_0  = 0x0B, // push 0.0 onto stack
    DCONST_1  = 0x0C, // push 1.0 onto stack
    DCONST_2  = 0x0D, // push 2.0 onto stack
    /* make sure you consider endianness */
    DCONST    = 0x0F, // push next 8 bytes onto stack as double constant
    /* used for controlling program flow */
    JEQ       = 0x10, // absolute jump to address in next 4 bytes if r1 == r2
    JNE       = 0x11, // absolute jump to address in next 4 bytes if r1 != r2
    JLT       = 0x12, // absolute jump to address in next 4 bytes if r1 <  r2
    JLE       = 0x13, // absolute jump to address in next 4 bytes if r1 <= r2
    JGT       = 0x14, // absolute jump to address in next 4 bytes if r1 >  r2
    JGE       = 0x15, // absolute jump to address in next 4 bytes if r1 >= r2
    /* arithmetic operations */
    ADD       = 0x60, // add two doubles
    SUB       = 0x61, // subtract two doubles
    MUL       = 0x62, // multiply two doubles
    DIV       = 0x64, // divide two doubles
    NEG       = 0x70, // negate an double (e.g. if -1.0 is on the stack, NEG will turn it to 1.0 on the stack)

    NOP       = 0xF0, // do nothing
    /* in a real VM, we'd use a function call for print, rather than having a special opcode */
    PRINT     = 0xF2, // pops and prints top of stack
    /* store and read from registers */
    ST1       = 0xF4, // pops top of stack and stores it in r1
    LD1       = 0xF5, // load global from r1
    ST2       = 0xF6, // pops top of stack and stores it in r2
    LD2       = 0xF7, // load global from r2
};

/* defining our virtual machine */
typedef struct {
    double r1, r2;      // registers
    char* code;         // pointer to bytecode
    double* stack;      // stack
    int pc;             // program counter
    int sp;             // stack pointer
} VM;

VM* newVM(char* code /* pointer to bytecode */ ) {
    VM* vm = malloc(sizeof(VM));
    vm->code = code; 
    vm->pc = 0;
    vm->sp = -1;
    vm->r1 = vm->r2 = 0;    // init registers to 0
    vm->stack = malloc(sizeof(double) * STACK_SIZE);
    return vm;
}

void delVM(VM* vm){
    free(vm->stack);
    free(vm);
}

void swap(char* p, int a, int b) {
    char temp = p[a];
    p[a] = p[b];
    p[b] = temp;
}

void convertToLittleEndian(void* v, size_t size) {
    char* p = (char*) v;
    for (size_t i = 0; i < size / 2; i++)
       swap(p, i, size - i - 1);
}

int run(VM* vm){
    for (;;) {
        unsigned char opcode = NCODE(vm);        // store next bytecode in `opcode'
        double a, b, v;                          // use these to store intermediate values when implementing opcodes below
        switch (opcode) {   // decode
        case HALT: return EXIT_SUCCESS;  // exit successfully
        case NOP: break;    // pass
        case DCONST_M1:     // push -1.0 onto stack
            PUSH(vm, -1);
            break;
        case DCONST_0:      // push 0.0 onto stack
            PUSH(vm, 0);
            break;
        case DCONST_1:      // push 1.0 onto stack
            PUSH(vm, 1);
            break;
        case DCONST_2:      // push 2.0 onto stack
            PUSH(vm, 2);
            break;
        case DCONST:        // reads next 8 bytes of opcode as a double, and stores it on the stack.
            memcpy(&v, vm->code + vm->pc, 8);
            vm->pc += 8;
#if LITTLE_ENDIAN
            convertToLittleEndian(&v, sizeof(double));
#endif
            PUSH(vm, v);
            break;
        case JEQ:           // absolute jump to address in next 4 bytes if r1 == r2
            if (vm->r1 == vm->r2) {
                JMP(vm);
            }
            break;
        case JNE:           // absolute jump to address in next 4 bytes if r1 != r2
            if (vm->r1 != vm->r2) {
                JMP(vm);
            }
            break;
        case JLT:           // absolute jump to address in next 4 bytes if r1 <  r2
            if (vm->r1 < vm->r2) {
                JMP(vm);
            }
            break;
        case JLE:           // absolute jump to address in next 4 bytes if r1 <= r2
            if (vm->r1 <= vm->r2) {
                JMP(vm);
            }
            break;
        case JGT:           // absolute jump to address in next 4 bytes if r1 >  r2
            if (vm->r1 > vm->r2) {
                JMP(vm);
            }
            break;
        case JGE:           // absolute jump to address in next 4 bytes if r1 >= r2
            if (vm->r1 >= vm->r2) {
                JMP(vm);
            }
            break;
        case ADD:           // add two doubles from top of stack and push result back onto stack
            b = POP(vm);
            a = POP(vm);
            PUSH(vm, a + b);
            break;
        case MUL:           // multiply two doubles from top of stack and push result back onto stack
            b = POP(vm);
            a = POP(vm);
            PUSH(vm, a * b);
            break;
        case SUB:           // subtract two doubles from top of stack and push result back onto stack
            b = POP(vm);
            a = POP(vm);
            PUSH(vm, a - b);
            break;
        case DIV:          // divide two doubles from top of stack and push result back onto stack
            b = POP(vm);
            a = POP(vm);

            // Terminate on division by zero
            if (b == 0) {
                printf("RuntimeException: Division by zero @ PC = %d\n", vm->pc);
                return EXIT_FAILURE;
            }
            PUSH(vm, a / b);
            break;
        case NEG:                         // negates top of stack
            v = POP(vm);
            PUSH(vm, -v);
            break;
        case LD1:          // put value from r1 on top of stack
            PUSH(vm, vm->r1);
            break;
        case ST1:                         // store top of stack in r1
            vm->r1 = POP(vm);
            break;
        case LD2:           // put value from r2 on top of stack
            PUSH(vm, vm->r2);
            break;
        case ST2:                         // store top of stack in r2
            vm->r2 = POP(vm);
            break;
        case PRINT:                       // print top of stack, (and discard value afterwards.)
            v = POP(vm);
            printf("%f\n", v);
            break;
        default:
            printf("InvalidOpcodeError: %x\n", opcode);  // terminate program at unknown opcode and show error.
            return EXIT_FAILURE;
        }

    }
    return EXIT_FAILURE;
}

int main(void) {
    /* in a real VM, we'd read bytecode from a file, but for brevity's sake we'll read
    from an array.
    */
    // simple example: push 2 onto stack, push 1 onto stack, subtract them, print the result, exit (should print 1.0)
    char bytecode[] = {
        DCONST_2, DCONST_1,
        SUB,
        PRINT,
        HALT };
    char readmebytecode[] = {
        // 12.54 double precision pushed on stack
        DCONST,
        0x40, 0x29, 0x14, 0x7A, 0xE1, 0x47, 0xAE, 0x14,

        PRINT,
        HALT
    };
    // Fibonacci program: prints the Fibonacci sequence ending when the last 2
    //                    numbers are both greater than 100.
    char fibonacciCode[] = {
        // Start everything with 0 and 1 already on the stack
        DCONST_0,
        DCONST_0,
        PRINT,
        DCONST_1,
        DCONST_1,
        PRINT,
        // Start of loop (index:6)
        // Load the 2 values
        ST2,
        ST1,
        // The 2 values are gone - push them back on
        LD1,
        LD2,
        // Add the 2 values at the top of the stack and save to variable
        ADD,
        ST1,
        LD1,
        LD1,
        PRINT,      // printing consumes one, so get an extra
        // Push the 2nd variable onto the stack and add, then save to variable
        LD2,
        ADD,
        ST2,
        LD2,
        PRINT,
        // Reload all variables back onto stack
        LD1,
        LD2,
        // Compare largest with set value "100" and loop if we haven't gotten
        // there yet
        DCONST,
        0x40, 0x59, 0, 0, 0, 0, 0, 0,       // 100.0
        ST1,        // the largest value is always in r2, so overwrite r1 to compare
        JGT,
        0, 0, 0, 6,                         // abs. addr. 2, or also, beginning of loop

        HALT,
    };
    //VM* vm = newVM(bytecode /* program to execute */ );
    VM* vm = newVM(fibonacciCode);
    int exit_status = run(vm);
    delVM(vm);
    return exit_status;
};
